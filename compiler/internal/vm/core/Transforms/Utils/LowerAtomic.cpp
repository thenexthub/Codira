//===- LowerAtomic.cpp - Lower atomic intrinsics --------------------------===//
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
// This pass lowers atomic intrinsics to non-atomic form for use in a known
// non-preemptible environment.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Transforms/Utils/LowerAtomic.h"
#include "vm/core/IR/Function.h"
#include "vm/core/IR/IRBuilder.h"

using namespace vm::core;

#define DEBUG_TYPE "loweratomic"

bool toolchain::lowerAtomicCmpXchgInst(AtomicCmpXchgInst *CXI) {
  IRBuilder<> Builder(CXI);
  Value *Ptr = CXI->getPointerOperand();
  Value *Cmp = CXI->getCompareOperand();
  Value *Val = CXI->getNewValOperand();

  auto [Orig, Equal] =
      buildCmpXchgValue(Builder, Ptr, Cmp, Val, CXI->getAlign());

  Value *Res =
      Builder.CreateInsertValue(PoisonValue::get(CXI->getType()), Orig, 0);
  Res = Builder.CreateInsertValue(Res, Equal, 1);

  CXI->replaceAllUsesWith(Res);
  CXI->eraseFromParent();
  return true;
}

std::pair<Value *, Value *> toolchain::buildCmpXchgValue(IRBuilderBase &Builder,
                                                    Value *Ptr, Value *Cmp,
                                                    Value *Val,
                                                    Align Alignment) {
  LoadInst *Orig = Builder.CreateAlignedLoad(Val->getType(), Ptr, Alignment);
  Value *Equal = Builder.CreateICmpEQ(Orig, Cmp);
  Value *Res = Builder.CreateSelect(Equal, Val, Orig);
  Builder.CreateAlignedStore(Res, Ptr, Alignment);

  return {Orig, Equal};
}

Value *toolchain::buildAtomicRMWValue(AtomicRMWInst::BinOp Op,
                                 IRBuilderBase &Builder, Value *Loaded,
                                 Value *Val) {
  Value *NewVal;
  switch (Op) {
  case AtomicRMWInst::Xchg:
    return Val;
  case AtomicRMWInst::Add:
    return Builder.CreateAdd(Loaded, Val, "new");
  case AtomicRMWInst::Sub:
    return Builder.CreateSub(Loaded, Val, "new");
  case AtomicRMWInst::And:
    return Builder.CreateAnd(Loaded, Val, "new");
  case AtomicRMWInst::Nand:
    return Builder.CreateNot(Builder.CreateAnd(Loaded, Val), "new");
  case AtomicRMWInst::Or:
    return Builder.CreateOr(Loaded, Val, "new");
  case AtomicRMWInst::Xor:
    return Builder.CreateXor(Loaded, Val, "new");
  case AtomicRMWInst::Max:
    NewVal = Builder.CreateICmpSGT(Loaded, Val);
    return Builder.CreateSelect(NewVal, Loaded, Val, "new");
  case AtomicRMWInst::Min:
    NewVal = Builder.CreateICmpSLE(Loaded, Val);
    return Builder.CreateSelect(NewVal, Loaded, Val, "new");
  case AtomicRMWInst::UMax:
    NewVal = Builder.CreateICmpUGT(Loaded, Val);
    return Builder.CreateSelect(NewVal, Loaded, Val, "new");
  case AtomicRMWInst::UMin:
    NewVal = Builder.CreateICmpULE(Loaded, Val);
    return Builder.CreateSelect(NewVal, Loaded, Val, "new");
  case AtomicRMWInst::FAdd:
    return Builder.CreateFAdd(Loaded, Val, "new");
  case AtomicRMWInst::FSub:
    return Builder.CreateFSub(Loaded, Val, "new");
  case AtomicRMWInst::FMax:
    return Builder.CreateMaxNum(Loaded, Val);
  case AtomicRMWInst::FMin:
    return Builder.CreateMinNum(Loaded, Val);
  case AtomicRMWInst::FMaximum:
    return Builder.CreateMaximum(Loaded, Val);
  case AtomicRMWInst::FMinimum:
    return Builder.CreateMinimum(Loaded, Val);
  case AtomicRMWInst::UIncWrap: {
    Constant *One = ConstantInt::get(Loaded->getType(), 1);
    Value *Inc = Builder.CreateAdd(Loaded, One);
    Value *Cmp = Builder.CreateICmpUGE(Loaded, Val);
    Constant *Zero = ConstantInt::get(Loaded->getType(), 0);
    return Builder.CreateSelect(Cmp, Zero, Inc, "new");
  }
  case AtomicRMWInst::UDecWrap: {
    Constant *Zero = ConstantInt::get(Loaded->getType(), 0);
    Constant *One = ConstantInt::get(Loaded->getType(), 1);

    Value *Dec = Builder.CreateSub(Loaded, One);
    Value *CmpEq0 = Builder.CreateICmpEQ(Loaded, Zero);
    Value *CmpOldGtVal = Builder.CreateICmpUGT(Loaded, Val);
    Value *Or = Builder.CreateOr(CmpEq0, CmpOldGtVal);
    return Builder.CreateSelect(Or, Val, Dec, "new");
  }
  case AtomicRMWInst::USubCond: {
    Value *Cmp = Builder.CreateICmpUGE(Loaded, Val);
    Value *Sub = Builder.CreateSub(Loaded, Val);
    return Builder.CreateSelect(Cmp, Sub, Loaded, "new");
  }
  case AtomicRMWInst::USubSat:
    return Builder.CreateIntrinsic(Intrinsic::usub_sat, Loaded->getType(),
                                   {Loaded, Val}, nullptr, "new");
  default:
    llvm_unreachable("Unknown atomic op");
  }
}

bool toolchain::lowerAtomicRMWInst(AtomicRMWInst *RMWI) {
  IRBuilder<> Builder(RMWI);
  Builder.setIsFPConstrained(
      RMWI->getFunction()->hasFnAttribute(Attribute::StrictFP));

  Value *Ptr = RMWI->getPointerOperand();
  Value *Val = RMWI->getValOperand();

  LoadInst *Orig = Builder.CreateLoad(Val->getType(), Ptr);
  Value *Res = buildAtomicRMWValue(RMWI->getOperation(), Builder, Orig, Val);
  Builder.CreateStore(Res, Ptr);
  RMWI->replaceAllUsesWith(Orig);
  RMWI->eraseFromParent();
  return true;
}
