//===-- RecordOps.cpp -------------------------------------------*- C++ -*-===//
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
//  Operations on records (structs, classes, and unions).
//
//===----------------------------------------------------------------------===//

#include "language/Core/Analysis/FlowSensitive/RecordOps.h"

#define DEBUG_TYPE "dataflow"

namespace language::Core::dataflow {

static void copyField(const ValueDecl &Field, StorageLocation *SrcFieldLoc,
                      StorageLocation *DstFieldLoc, RecordStorageLocation &Dst,
                      Environment &Env) {
  assert(Field.getType()->isReferenceType() ||
         (SrcFieldLoc != nullptr && DstFieldLoc != nullptr));

  if (Field.getType()->isRecordType()) {
    copyRecord(cast<RecordStorageLocation>(*SrcFieldLoc),
               cast<RecordStorageLocation>(*DstFieldLoc), Env);
  } else if (Field.getType()->isReferenceType()) {
    Dst.setChild(Field, SrcFieldLoc);
  } else {
    if (Value *Val = Env.getValue(*SrcFieldLoc))
      Env.setValue(*DstFieldLoc, *Val);
    else
      Env.clearValue(*DstFieldLoc);
  }
}

static void copySyntheticField(QualType FieldType, StorageLocation &SrcFieldLoc,
                               StorageLocation &DstFieldLoc, Environment &Env) {
  if (FieldType->isRecordType()) {
    copyRecord(cast<RecordStorageLocation>(SrcFieldLoc),
               cast<RecordStorageLocation>(DstFieldLoc), Env);
  } else {
    if (Value *Val = Env.getValue(SrcFieldLoc))
      Env.setValue(DstFieldLoc, *Val);
    else
      Env.clearValue(DstFieldLoc);
  }
}

void copyRecord(RecordStorageLocation &Src, RecordStorageLocation &Dst,
                Environment &Env) {
  auto SrcType = Src.getType().getCanonicalType().getUnqualifiedType();
  auto DstType = Dst.getType().getCanonicalType().getUnqualifiedType();

  auto SrcDecl = SrcType->getAsCXXRecordDecl();
  auto DstDecl = DstType->getAsCXXRecordDecl();

  [[maybe_unused]] bool compatibleTypes =
      SrcType == DstType ||
      (SrcDecl != nullptr && DstDecl != nullptr &&
       (SrcDecl->isDerivedFrom(DstDecl) || DstDecl->isDerivedFrom(SrcDecl)));

  LLVM_DEBUG({
    if (!compatibleTypes) {
      toolchain::dbgs() << "Source type " << Src.getType() << "\n";
      toolchain::dbgs() << "Destination type " << Dst.getType() << "\n";
    }
  });
  assert(compatibleTypes);

  if (SrcType == DstType || (SrcDecl != nullptr && DstDecl != nullptr &&
                             SrcDecl->isDerivedFrom(DstDecl))) {
    for (auto [Field, DstFieldLoc] : Dst.children())
      copyField(*Field, Src.getChild(*Field), DstFieldLoc, Dst, Env);
    for (const auto &[Name, DstFieldLoc] : Dst.synthetic_fields())
      copySyntheticField(DstFieldLoc->getType(), Src.getSyntheticField(Name),
                         *DstFieldLoc, Env);
  } else {
    for (auto [Field, SrcFieldLoc] : Src.children())
      copyField(*Field, SrcFieldLoc, Dst.getChild(*Field), Dst, Env);
    for (const auto &[Name, SrcFieldLoc] : Src.synthetic_fields())
      copySyntheticField(SrcFieldLoc->getType(), *SrcFieldLoc,
                         Dst.getSyntheticField(Name), Env);
  }
}

bool recordsEqual(const RecordStorageLocation &Loc1, const Environment &Env1,
                  const RecordStorageLocation &Loc2, const Environment &Env2) {
  LLVM_DEBUG({
    if (Loc2.getType().getCanonicalType().getUnqualifiedType() !=
        Loc1.getType().getCanonicalType().getUnqualifiedType()) {
      toolchain::dbgs() << "Loc1 type " << Loc1.getType() << "\n";
      toolchain::dbgs() << "Loc2 type " << Loc2.getType() << "\n";
    }
  });
  assert(Loc2.getType().getCanonicalType().getUnqualifiedType() ==
         Loc1.getType().getCanonicalType().getUnqualifiedType());

  for (auto [Field, FieldLoc1] : Loc1.children()) {
    StorageLocation *FieldLoc2 = Loc2.getChild(*Field);

    assert(Field->getType()->isReferenceType() ||
           (FieldLoc1 != nullptr && FieldLoc2 != nullptr));

    if (Field->getType()->isRecordType()) {
      if (!recordsEqual(cast<RecordStorageLocation>(*FieldLoc1), Env1,
                        cast<RecordStorageLocation>(*FieldLoc2), Env2))
        return false;
    } else if (Field->getType()->isReferenceType()) {
      if (FieldLoc1 != FieldLoc2)
        return false;
    } else if (Env1.getValue(*FieldLoc1) != Env2.getValue(*FieldLoc2)) {
      return false;
    }
  }

  for (const auto &[Name, SynthFieldLoc1] : Loc1.synthetic_fields()) {
    if (SynthFieldLoc1->getType()->isRecordType()) {
      if (!recordsEqual(
              *cast<RecordStorageLocation>(SynthFieldLoc1), Env1,
              cast<RecordStorageLocation>(Loc2.getSyntheticField(Name)), Env2))
        return false;
    } else if (Env1.getValue(*SynthFieldLoc1) !=
               Env2.getValue(Loc2.getSyntheticField(Name))) {
      return false;
    }
  }

  return true;
}

} // namespace language::Core::dataflow
