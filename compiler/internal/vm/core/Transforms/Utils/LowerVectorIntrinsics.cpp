//===- LowerVectorIntrinsics.cpp ------------------------------------------===//
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

#include "vm/core/Transforms/Utils/LowerVectorIntrinsics.h"
#include "vm/core/IR/IRBuilder.h"

#define DEBUG_TYPE "lower-vector-intrinsics"

using namespace vm::core;

bool toolchain::lowerUnaryVectorIntrinsicAsLoop(Module &M, CallInst *CI) {
  Type *ArgTy = CI->getArgOperand(0)->getType();
  VectorType *VecTy = cast<VectorType>(ArgTy);

  BasicBlock *PreLoopBB = CI->getParent();
  BasicBlock *PostLoopBB = nullptr;
  Function *ParentFunc = PreLoopBB->getParent();
  LLVMContext &Ctx = PreLoopBB->getContext();
  Type *Int64Ty = IntegerType::get(Ctx, 64);

  PostLoopBB = PreLoopBB->splitBasicBlock(CI);
  BasicBlock *LoopBB = BasicBlock::Create(Ctx, "", ParentFunc, PostLoopBB);
  PreLoopBB->getTerminator()->setSuccessor(0, LoopBB);

  // Loop preheader
  IRBuilder<> PreLoopBuilder(PreLoopBB->getTerminator());
  Value *LoopEnd =
      PreLoopBuilder.CreateElementCount(Int64Ty, VecTy->getElementCount());

  // Loop body
  IRBuilder<> LoopBuilder(LoopBB);

  PHINode *LoopIndex = LoopBuilder.CreatePHI(Int64Ty, 2);
  LoopIndex->addIncoming(ConstantInt::get(Int64Ty, 0U), PreLoopBB);
  PHINode *Vec = LoopBuilder.CreatePHI(VecTy, 2);
  Vec->addIncoming(CI->getArgOperand(0), PreLoopBB);

  Value *Elem = LoopBuilder.CreateExtractElement(Vec, LoopIndex);
  Function *Exp = Intrinsic::getOrInsertDeclaration(&M, CI->getIntrinsicID(),
                                                    VecTy->getElementType());
  Value *Res = LoopBuilder.CreateCall(Exp, Elem);
  Value *NewVec = LoopBuilder.CreateInsertElement(Vec, Res, LoopIndex);
  Vec->addIncoming(NewVec, LoopBB);

  Value *One = ConstantInt::get(Int64Ty, 1U);
  Value *NextLoopIndex = LoopBuilder.CreateAdd(LoopIndex, One);
  LoopIndex->addIncoming(NextLoopIndex, LoopBB);

  Value *ExitCond =
      LoopBuilder.CreateICmp(CmpInst::ICMP_EQ, NextLoopIndex, LoopEnd);
  LoopBuilder.CreateCondBr(ExitCond, PostLoopBB, LoopBB);

  CI->replaceAllUsesWith(NewVec);
  CI->eraseFromParent();
  return true;
}
