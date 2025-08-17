//===- RecordLayout.cpp - Layout information for a struct/union -----------===//
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
//  This file defines the RecordLayout interface.
//
//===----------------------------------------------------------------------===//

#include "language/Core/AST/RecordLayout.h"
#include "language/Core/AST/ASTContext.h"
#include "language/Core/Basic/TargetCXXABI.h"
#include "language/Core/Basic/TargetInfo.h"
#include <cassert>

using namespace language::Core;

void ASTRecordLayout::Destroy(ASTContext &Ctx) {
  if (CXXInfo) {
    CXXInfo->~CXXRecordLayoutInfo();
    Ctx.Deallocate(CXXInfo);
  }
  this->~ASTRecordLayout();
  Ctx.Deallocate(this);
}

ASTRecordLayout::ASTRecordLayout(const ASTContext &Ctx, CharUnits size,
                                 CharUnits alignment,
                                 CharUnits preferredAlignment,
                                 CharUnits unadjustedAlignment,
                                 CharUnits requiredAlignment,
                                 CharUnits datasize,
                                 ArrayRef<uint64_t> fieldoffsets)
    : Size(size), DataSize(datasize), Alignment(alignment),
      PreferredAlignment(preferredAlignment),
      UnadjustedAlignment(unadjustedAlignment),
      RequiredAlignment(requiredAlignment) {
  FieldOffsets.append(Ctx, fieldoffsets.begin(), fieldoffsets.end());
}

// Constructor for C++ records.
ASTRecordLayout::ASTRecordLayout(
    const ASTContext &Ctx, CharUnits size, CharUnits alignment,
    CharUnits preferredAlignment, CharUnits unadjustedAlignment,
    CharUnits requiredAlignment, bool hasOwnVFPtr, bool hasExtendableVFPtr,
    CharUnits vbptroffset, CharUnits datasize, ArrayRef<uint64_t> fieldoffsets,
    CharUnits nonvirtualsize, CharUnits nonvirtualalignment,
    CharUnits preferrednvalignment, CharUnits SizeOfLargestEmptySubobject,
    const CXXRecordDecl *PrimaryBase, bool IsPrimaryBaseVirtual,
    const CXXRecordDecl *BaseSharingVBPtr, bool EndsWithZeroSizedObject,
    bool LeadsWithZeroSizedBase, const BaseOffsetsMapTy &BaseOffsets,
    const VBaseOffsetsMapTy &VBaseOffsets)
    : Size(size), DataSize(datasize), Alignment(alignment),
      PreferredAlignment(preferredAlignment),
      UnadjustedAlignment(unadjustedAlignment),
      RequiredAlignment(requiredAlignment),
      CXXInfo(new (Ctx) CXXRecordLayoutInfo) {
  FieldOffsets.append(Ctx, fieldoffsets.begin(), fieldoffsets.end());

  CXXInfo->PrimaryBase.setPointer(PrimaryBase);
  CXXInfo->PrimaryBase.setInt(IsPrimaryBaseVirtual);
  CXXInfo->NonVirtualSize = nonvirtualsize;
  CXXInfo->NonVirtualAlignment = nonvirtualalignment;
  CXXInfo->PreferredNVAlignment = preferrednvalignment;
  CXXInfo->SizeOfLargestEmptySubobject = SizeOfLargestEmptySubobject;
  CXXInfo->BaseOffsets = BaseOffsets;
  CXXInfo->VBaseOffsets = VBaseOffsets;
  CXXInfo->HasOwnVFPtr = hasOwnVFPtr;
  CXXInfo->VBPtrOffset = vbptroffset;
  CXXInfo->HasExtendableVFPtr = hasExtendableVFPtr;
  CXXInfo->BaseSharingVBPtr = BaseSharingVBPtr;
  CXXInfo->EndsWithZeroSizedObject = EndsWithZeroSizedObject;
  CXXInfo->LeadsWithZeroSizedBase = LeadsWithZeroSizedBase;

#ifndef NDEBUG
    if (const CXXRecordDecl *PrimaryBase = getPrimaryBase()) {
      if (isPrimaryBaseVirtual()) {
        if (Ctx.getTargetInfo().getCXXABI().hasPrimaryVBases()) {
          assert(getVBaseClassOffset(PrimaryBase).isZero() &&
                 "Primary virtual base must be at offset 0!");
        }
      } else {
        assert(getBaseClassOffset(PrimaryBase).isZero() &&
               "Primary base must be at offset 0!");
      }
    }
#endif
}
