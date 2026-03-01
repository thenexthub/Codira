//===- MemoryModelRelaxationAnnotations.cpp ---------------------*- C++ -*-===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
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

#include "vm/core/IR/MemoryModelRelaxationAnnotations.h"
#include "vm/core/IR/Instructions.h"
#include "vm/core/IR/Metadata.h"
#include "vm/core/Support/Debug.h"
#include "vm/core/Support/raw_ostream.h"

using namespace vm::core;

//===- MMRAMetadata -------------------------------------------------------===//

MMRAMetadata::MMRAMetadata(const Instruction &I)
    : MMRAMetadata(I.getMetadata(LLVMContext::MD_mmra)) {}

MMRAMetadata::MMRAMetadata(MDNode *MD) {
  if (!MD)
    return;

  // TODO: Split this into a "tryParse" function that can return an err.
  // CTor can use the tryParse & just fatal on err.

  MDTuple *Tuple = dyn_cast<MDTuple>(MD);
  assert(Tuple && "Invalid MMRA structure");

  const auto HandleTagMD = [this](MDNode *TagMD) {
    Tags.insert({cast<MDString>(TagMD->getOperand(0))->getString(),
                 cast<MDString>(TagMD->getOperand(1))->getString()});
  };

  if (isTagMD(Tuple)) {
    HandleTagMD(Tuple);
    return;
  }

  for (const MDOperand &Op : Tuple->operands()) {
    MDNode *MDOp = cast<MDNode>(Op.get());
    assert(isTagMD(MDOp));
    HandleTagMD(MDOp);
  }
}

bool MMRAMetadata::isTagMD(const Metadata *MD) {
  if (auto *Tuple = dyn_cast<MDTuple>(MD)) {
    return Tuple->getNumOperands() == 2 &&
           isa<MDString>(Tuple->getOperand(0)) &&
           isa<MDString>(Tuple->getOperand(1));
  }
  return false;
}

MDTuple *MMRAMetadata::getTagMD(LLVMContext &Ctx, StringRef Prefix,
                                StringRef Suffix) {
  return MDTuple::get(Ctx,
                      {MDString::get(Ctx, Prefix), MDString::get(Ctx, Suffix)});
}

MDTuple *MMRAMetadata::getMD(LLVMContext &Ctx,
                             ArrayRef<MMRAMetadata::TagT> Tags) {
  if (Tags.empty())
    return nullptr;

  if (Tags.size() == 1)
    return getTagMD(Ctx, Tags.front());

  SmallVector<Metadata *> MMRAs;
  for (const auto &Tag : Tags)
    MMRAs.push_back(getTagMD(Ctx, Tag));
  return MDTuple::get(Ctx, MMRAs);
}

MDNode *MMRAMetadata::combine(LLVMContext &Ctx, const MMRAMetadata &A,
                              const MMRAMetadata &B) {
  // Let A and B be two tags set, and U be the prefix-wise union of A and B.
  // For every unique tag prefix P present in A or B:
  // * If either A or B has no tags with prefix P, no tags with prefix
  //   P are added to U.
  // * If both A and B have at least one tag with prefix P, all tags with prefix
  //   P from both sets are added to U.

  SmallVector<Metadata *> Result;

  for (const auto &[P, S] : A) {
    if (B.hasTagWithPrefix(P))
      Result.push_back(getTagMD(Ctx, P, S));
  }
  for (const auto &[P, S] : B) {
    if (A.hasTagWithPrefix(P))
      Result.push_back(getTagMD(Ctx, P, S));
  }

  return MDTuple::get(Ctx, Result);
}

bool MMRAMetadata::hasTag(StringRef Prefix, StringRef Suffix) const {
  return Tags.count({Prefix, Suffix});
}

bool MMRAMetadata::isCompatibleWith(const MMRAMetadata &Other) const {
  // Two sets of tags are compatible iff, for every unique tag prefix P
  // present in at least one set:
  //   - the other set contains no tag with prefix P, or
  //   - at least one tag with prefix P is common to both sets.

  StringMap<bool> PrefixStatuses;
  for (const auto &[P, S] : Tags)
    PrefixStatuses[P] |= (Other.hasTag(P, S) || !Other.hasTagWithPrefix(P));
  for (const auto &[P, S] : Other)
    PrefixStatuses[P] |= (hasTag(P, S) || !hasTagWithPrefix(P));

  for (auto &[Prefix, Status] : PrefixStatuses) {
    if (!Status)
      return false;
  }

  return true;
}

bool MMRAMetadata::hasTagWithPrefix(StringRef Prefix) const {
  for (const auto &[P, S] : Tags)
    if (P == Prefix)
      return true;
  return false;
}

MMRAMetadata::const_iterator MMRAMetadata::begin() const {
  return Tags.begin();
}

MMRAMetadata::const_iterator MMRAMetadata::end() const { return Tags.end(); }

bool MMRAMetadata::empty() const { return Tags.empty(); }

unsigned MMRAMetadata::size() const { return Tags.size(); }

void MMRAMetadata::print(raw_ostream &OS) const {
  bool IsFirst = true;
  // TODO: use map_iter + join
  for (const auto &[P, S] : Tags) {
    if (IsFirst)
      IsFirst = false;
    else
      OS << ", ";
    OS << P << ":" << S;
  }
}

LLVM_DUMP_METHOD
void MMRAMetadata::dump() const { print(dbgs()); }

//===- Helpers ------------------------------------------------------------===//

static bool isReadWriteMemCall(const Instruction &I) {
  if (const auto *C = dyn_cast<CallBase>(&I))
    return C->mayReadOrWriteMemory() ||
           !C->getMemoryEffects().doesNotAccessMemory();
  return false;
}

bool toolchain::canInstructionHaveMMRAs(const Instruction &I) {
  return isa<LoadInst>(I) || isa<StoreInst>(I) || isa<AtomicCmpXchgInst>(I) ||
         isa<AtomicRMWInst>(I) || isa<FenceInst>(I) || isReadWriteMemCall(I);
}
