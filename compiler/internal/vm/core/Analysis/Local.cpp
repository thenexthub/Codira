//===- Local.cpp - Functions to perform local transformations -------------===//
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
//
// This family of functions perform various local transformations to the
// program.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Analysis/Utils/Local.h"
#include "vm/core/ADT/Twine.h"
#include "vm/core/IR/DataLayout.h"
#include "vm/core/IR/GetElementPtrTypeIterator.h"
#include "vm/core/IR/IRBuilder.h"

using namespace vm::core;

Value *toolchain::emitGEPOffset(IRBuilderBase *Builder, const DataLayout &DL,
                           User *GEP, bool NoAssumptions) {
  GEPOperator *GEPOp = cast<GEPOperator>(GEP);
  Type *IntIdxTy = DL.getIndexType(GEP->getType());
  Value *Result = nullptr;

  // nusw implies nsw for the offset arithmetic.
  bool NSW = GEPOp->hasNoUnsignedSignedWrap() && !NoAssumptions;
  bool NUW = GEPOp->hasNoUnsignedWrap() && !NoAssumptions;
  auto AddOffset = [&](Value *Offset) {
    if (Result)
      Result = Builder->CreateAdd(Result, Offset, GEP->getName() + ".offs",
                                  NUW, NSW);
    else
      Result = Offset;
  };

  gep_type_iterator GTI = gep_type_begin(GEP);
  for (User::op_iterator i = GEP->op_begin() + 1, e = GEP->op_end(); i != e;
       ++i, ++GTI) {
    Value *Op = *i;
    if (Constant *OpC = dyn_cast<Constant>(Op)) {
      if (OpC->isZeroValue())
        continue;

      // Handle a struct index, which adds its field offset to the pointer.
      if (StructType *STy = GTI.getStructTypeOrNull()) {
        uint64_t OpValue = OpC->getUniqueInteger().getZExtValue();
        uint64_t Size = DL.getStructLayout(STy)->getElementOffset(OpValue);
        if (!Size)
          continue;

        AddOffset(ConstantInt::get(IntIdxTy, Size));
        continue;
      }
    }

    // Splat the index if needed.
    if (IntIdxTy->isVectorTy() && !Op->getType()->isVectorTy())
      Op = Builder->CreateVectorSplat(
          cast<VectorType>(IntIdxTy)->getElementCount(), Op);

    // Convert to correct type.
    if (Op->getType() != IntIdxTy)
      Op = Builder->CreateIntCast(Op, IntIdxTy, true, Op->getName() + ".c");
    TypeSize TSize = GTI.getSequentialElementStride(DL);
    if (TSize != TypeSize::getFixed(1)) {
      Value *Scale = Builder->CreateTypeSize(IntIdxTy->getScalarType(), TSize);
      if (IntIdxTy->isVectorTy())
        Scale = Builder->CreateVectorSplat(
            cast<VectorType>(IntIdxTy)->getElementCount(), Scale);
      // We'll let instcombine(mul) convert this to a shl if possible.
      Op = Builder->CreateMul(Op, Scale, GEP->getName() + ".idx", NUW, NSW);
    }
    AddOffset(Op);
  }
  return Result ? Result : Constant::getNullValue(IntIdxTy);
}
