//===--- Randstruct.cpp ---------------------------------------------------===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
// 
// Author: Tunjay Akbarli
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// 
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//
//
// This file contains the implementation for Clang's structure field layout
// randomization.
//
//===----------------------------------------------------------------------===//

#include "language/Core/AST/Randstruct.h"
#include "language/Core/AST/ASTContext.h"
#include "language/Core/AST/Attr.h"
#include "language/Core/AST/Decl.h"
#include "language/Core/AST/DeclCXX.h" // For StaticAssertDecl
#include "language/Core/Basic/Diagnostic.h"
#include "toolchain/ADT/SmallVector.h"

#include <algorithm>
#include <random>
#include <set>
#include <string>

using language::Core::ASTContext;
using language::Core::FieldDecl;
using toolchain::SmallVector;

namespace {

// FIXME: Replace this with some discovery once that mechanism exists.
enum { CACHE_LINE = 64 };

// The Bucket class holds the struct fields we're trying to fill to a
// cache-line.
class Bucket {
  SmallVector<FieldDecl *, 64> Fields;
  int Size = 0;

public:
  virtual ~Bucket() = default;

  SmallVector<FieldDecl *, 64> &fields() { return Fields; }
  void addField(FieldDecl *Field, int FieldSize);
  virtual bool canFit(int FieldSize) const {
    return Size + FieldSize <= CACHE_LINE;
  }
  virtual bool isBitfieldRun() const { return false; }
  bool full() const { return Size >= CACHE_LINE; }
};

void Bucket::addField(FieldDecl *Field, int FieldSize) {
  Size += FieldSize;
  Fields.push_back(Field);
}

struct BitfieldRunBucket : public Bucket {
  bool canFit(int FieldSize) const override { return true; }
  bool isBitfieldRun() const override { return true; }
};

void randomizeStructureLayoutImpl(const ASTContext &Context,
                                  toolchain::SmallVectorImpl<FieldDecl *> &FieldsOut,
                                  std::mt19937 &RNG) {
  // All of the Buckets produced by best-effort cache-line algorithm.
  SmallVector<std::unique_ptr<Bucket>, 16> Buckets;

  // The current bucket of fields that we are trying to fill to a cache-line.
  std::unique_ptr<Bucket> CurrentBucket;

  // The current bucket containing the run of adjacent bitfields to ensure they
  // remain adjacent.
  std::unique_ptr<BitfieldRunBucket> CurrentBitfieldRun;

  // Tracks the number of fields that we failed to fit to the current bucket,
  // and thus still need to be added later.
  size_t Skipped = 0;

  while (!FieldsOut.empty()) {
    // If we've Skipped more fields than we have remaining to place, that means
    // that they can't fit in our current bucket, and we need to start a new
    // one.
    if (Skipped >= FieldsOut.size()) {
      Skipped = 0;
      Buckets.push_back(std::move(CurrentBucket));
    }

    // Take the first field that needs to be put in a bucket.
    auto FieldIter = FieldsOut.begin();
    FieldDecl *FD = *FieldIter;

    if (FD->isBitField() && !FD->isZeroLengthBitField()) {
      // Start a bitfield run if this is the first bitfield we have found.
      if (!CurrentBitfieldRun)
        CurrentBitfieldRun = std::make_unique<BitfieldRunBucket>();

      // We've placed the field, and can remove it from the "awaiting Buckets"
      // vector called "Fields."
      CurrentBitfieldRun->addField(FD, /*FieldSize is irrelevant here*/ 1);
      FieldsOut.erase(FieldIter);
      continue;
    }

    // Else, current field is not a bitfield. If we were previously in a
    // bitfield run, end it.
    if (CurrentBitfieldRun)
      Buckets.push_back(std::move(CurrentBitfieldRun));

    // If we don't have a bucket, make one.
    if (!CurrentBucket)
      CurrentBucket = std::make_unique<Bucket>();

    uint64_t Width = Context.getTypeInfo(FD->getType()).Width;
    if (Width >= CACHE_LINE) {
      std::unique_ptr<Bucket> OverSized = std::make_unique<Bucket>();
      OverSized->addField(FD, Width);
      FieldsOut.erase(FieldIter);
      Buckets.push_back(std::move(OverSized));
      continue;
    }

    // If it fits, add it.
    if (CurrentBucket->canFit(Width)) {
      CurrentBucket->addField(FD, Width);
      FieldsOut.erase(FieldIter);

      // If it's now full, tie off the bucket.
      if (CurrentBucket->full()) {
        Skipped = 0;
        Buckets.push_back(std::move(CurrentBucket));
      }
    } else {
      // We can't fit it in our current bucket. Move to the end for processing
      // later.
      ++Skipped; // Mark it skipped.
      FieldsOut.push_back(FD);
      FieldsOut.erase(FieldIter);
    }
  }

  // Done processing the fields awaiting a bucket.

  // If we were filling a bucket, tie it off.
  if (CurrentBucket)
    Buckets.push_back(std::move(CurrentBucket));

  // If we were processing a bitfield run bucket, tie it off.
  if (CurrentBitfieldRun)
    Buckets.push_back(std::move(CurrentBitfieldRun));

  std::shuffle(std::begin(Buckets), std::end(Buckets), RNG);

  // Produce the new ordering of the elements from the Buckets.
  SmallVector<FieldDecl *, 16> FinalOrder;
  for (const std::unique_ptr<Bucket> &B : Buckets) {
    toolchain::SmallVectorImpl<FieldDecl *> &RandFields = B->fields();
    if (!B->isBitfieldRun())
      std::shuffle(std::begin(RandFields), std::end(RandFields), RNG);

    toolchain::append_range(FinalOrder, RandFields);
  }

  FieldsOut = FinalOrder;
}

} // anonymous namespace

namespace language::Core {
namespace randstruct {

bool randomizeStructureLayout(const ASTContext &Context, RecordDecl *RD,
                              SmallVectorImpl<Decl *> &FinalOrdering) {
  SmallVector<FieldDecl *, 64> RandomizedFields;
  SmallVector<Decl *, 8> PostRandomizedFields;

  unsigned TotalNumFields = 0;
  for (Decl *D : RD->decls()) {
    ++TotalNumFields;
    if (auto *FD = dyn_cast<FieldDecl>(D))
      RandomizedFields.push_back(FD);
    else if (isa<StaticAssertDecl>(D) || isa<IndirectFieldDecl>(D))
      PostRandomizedFields.push_back(D);
    else
      FinalOrdering.push_back(D);
  }

  if (RandomizedFields.empty())
    return false;

  // Struct might end with a flexible array or an array of size 0 or 1,
  // in which case we don't want to randomize it.
  FieldDecl *FlexibleArray =
      RD->hasFlexibleArrayMember() ? RandomizedFields.pop_back_val() : nullptr;
  if (!FlexibleArray) {
    if (const auto *CA =
            dyn_cast<ConstantArrayType>(RandomizedFields.back()->getType()))
      if (CA->getSize().sle(2))
        FlexibleArray = RandomizedFields.pop_back_val();
  }

  std::string Seed =
      Context.getLangOpts().RandstructSeed + RD->getNameAsString();
  std::seed_seq SeedSeq(Seed.begin(), Seed.end());
  std::mt19937 RNG(SeedSeq);

  randomizeStructureLayoutImpl(Context, RandomizedFields, RNG);

  // Plorp the randomized decls into the final ordering.
  toolchain::append_range(FinalOrdering, RandomizedFields);

  // Add fields that belong towards the end of the RecordDecl.
  toolchain::append_range(FinalOrdering, PostRandomizedFields);

  // Add back the flexible array.
  if (FlexibleArray)
    FinalOrdering.push_back(FlexibleArray);

  assert(TotalNumFields == FinalOrdering.size() &&
         "Decl count has been altered after Randstruct randomization!");
  (void)TotalNumFields;
  return true;
}

} // end namespace randstruct
} // end namespace language::Core
