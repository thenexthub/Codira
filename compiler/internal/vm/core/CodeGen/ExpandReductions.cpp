//===- ExpandReductions.cpp - Expand reduction intrinsics -----------------===//
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
// This pass implements IR expansion for reduction intrinsics, allowing targets
// to enable the intrinsics until just before codegen.
//
//===----------------------------------------------------------------------===//

#include "vm/core/CodeGen/ExpandReductions.h"
#include "vm/core/Analysis/TargetTransformInfo.h"
#include "vm/core/CodeGen/Passes.h"
#include "vm/core/IR/IRBuilder.h"
#include "vm/core/IR/InstIterator.h"
#include "vm/core/IR/IntrinsicInst.h"
#include "vm/core/IR/Intrinsics.h"
#include "vm/core/InitializePasses.h"
#include "vm/core/Pass.h"
#include "vm/core/Transforms/Utils/LoopUtils.h"

using namespace vm::core;

namespace {

bool expandReductions(Function &F, const TargetTransformInfo *TTI) {
  bool Changed = false;
  SmallVector<IntrinsicInst *, 4> Worklist;
  for (auto &I : instructions(F)) {
    if (auto *II = dyn_cast<IntrinsicInst>(&I)) {
      switch (II->getIntrinsicID()) {
      default: break;
      case Intrinsic::vector_reduce_fadd:
      case Intrinsic::vector_reduce_fmul:
      case Intrinsic::vector_reduce_add:
      case Intrinsic::vector_reduce_mul:
      case Intrinsic::vector_reduce_and:
      case Intrinsic::vector_reduce_or:
      case Intrinsic::vector_reduce_xor:
      case Intrinsic::vector_reduce_smax:
      case Intrinsic::vector_reduce_smin:
      case Intrinsic::vector_reduce_umax:
      case Intrinsic::vector_reduce_umin:
      case Intrinsic::vector_reduce_fmax:
      case Intrinsic::vector_reduce_fmin:
        if (TTI->shouldExpandReduction(II))
          Worklist.push_back(II);

        break;
      }
    }
  }

  for (auto *II : Worklist) {
    FastMathFlags FMF =
        isa<FPMathOperator>(II) ? II->getFastMathFlags() : FastMathFlags{};
    Intrinsic::ID ID = II->getIntrinsicID();
    RecurKind RK = getMinMaxReductionRecurKind(ID);
    TargetTransformInfo::ReductionShuffle RS =
        TTI->getPreferredExpandedReductionShuffle(II);

    Value *Rdx = nullptr;
    IRBuilder<> Builder(II);
    IRBuilder<>::FastMathFlagGuard FMFGuard(Builder);
    Builder.setFastMathFlags(FMF);
    switch (ID) {
    default: llvm_unreachable("Unexpected intrinsic!");
    case Intrinsic::vector_reduce_fadd:
    case Intrinsic::vector_reduce_fmul: {
      // FMFs must be attached to the call, otherwise it's an ordered reduction
      // and it can't be handled by generating a shuffle sequence.
      Value *Acc = II->getArgOperand(0);
      Value *Vec = II->getArgOperand(1);
      unsigned RdxOpcode = getArithmeticReductionInstruction(ID);
      if (!FMF.allowReassoc())
        Rdx = getOrderedReduction(Builder, Acc, Vec, RdxOpcode, RK);
      else {
        if (!isPowerOf2_32(
                cast<FixedVectorType>(Vec->getType())->getNumElements()))
          continue;
        Rdx = getShuffleReduction(Builder, Vec, RdxOpcode, RS, RK);
        Rdx = Builder.CreateBinOp((Instruction::BinaryOps)RdxOpcode, Acc, Rdx,
                                  "bin.rdx");
      }
      break;
    }
    case Intrinsic::vector_reduce_and:
    case Intrinsic::vector_reduce_or: {
      // Canonicalize logical or/and reductions:
      // Or reduction for i1 is represented as:
      // %val = bitcast <ReduxWidth x i1> to iReduxWidth
      // %res = cmp ne iReduxWidth %val, 0
      // And reduction for i1 is represented as:
      // %val = bitcast <ReduxWidth x i1> to iReduxWidth
      // %res = cmp eq iReduxWidth %val, 11111
      Value *Vec = II->getArgOperand(0);
      auto *FTy = cast<FixedVectorType>(Vec->getType());
      unsigned NumElts = FTy->getNumElements();
      if (!isPowerOf2_32(NumElts))
        continue;

      if (FTy->getElementType() == Builder.getInt1Ty()) {
        Rdx = Builder.CreateBitCast(Vec, Builder.getIntNTy(NumElts));
        if (ID == Intrinsic::vector_reduce_and) {
          Rdx = Builder.CreateICmpEQ(
              Rdx, ConstantInt::getAllOnesValue(Rdx->getType()));
        } else {
          assert(ID == Intrinsic::vector_reduce_or && "Expected or reduction.");
          Rdx = Builder.CreateIsNotNull(Rdx);
        }
        break;
      }
      unsigned RdxOpcode = getArithmeticReductionInstruction(ID);
      Rdx = getShuffleReduction(Builder, Vec, RdxOpcode, RS, RK);
      break;
    }
    case Intrinsic::vector_reduce_add:
    case Intrinsic::vector_reduce_mul:
    case Intrinsic::vector_reduce_xor:
    case Intrinsic::vector_reduce_smax:
    case Intrinsic::vector_reduce_smin:
    case Intrinsic::vector_reduce_umax:
    case Intrinsic::vector_reduce_umin: {
      Value *Vec = II->getArgOperand(0);
      if (!isPowerOf2_32(
              cast<FixedVectorType>(Vec->getType())->getNumElements()))
        continue;
      unsigned RdxOpcode = getArithmeticReductionInstruction(ID);
      Rdx = getShuffleReduction(Builder, Vec, RdxOpcode, RS, RK);
      break;
    }
    case Intrinsic::vector_reduce_fmax:
    case Intrinsic::vector_reduce_fmin: {
      // We require "nnan" to use a shuffle reduction; "nsz" is implied by the
      // semantics of the reduction.
      Value *Vec = II->getArgOperand(0);
      if (!isPowerOf2_32(
              cast<FixedVectorType>(Vec->getType())->getNumElements()) ||
          !FMF.noNaNs())
        continue;
      unsigned RdxOpcode = getArithmeticReductionInstruction(ID);
      Rdx = getShuffleReduction(Builder, Vec, RdxOpcode, RS, RK);
      break;
    }
    }
    II->replaceAllUsesWith(Rdx);
    II->eraseFromParent();
    Changed = true;
  }
  return Changed;
}

class ExpandReductions : public FunctionPass {
public:
  static char ID;
  ExpandReductions() : FunctionPass(ID) {
    initializeExpandReductionsPass(*PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &F) override {
    const auto *TTI =&getAnalysis<TargetTransformInfoWrapperPass>().getTTI(F);
    return expandReductions(F, TTI);
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<TargetTransformInfoWrapperPass>();
    AU.setPreservesCFG();
  }
};
}

char ExpandReductions::ID;
INITIALIZE_PASS_BEGIN(ExpandReductions, "expand-reductions",
                      "Expand reduction intrinsics", false, false)
INITIALIZE_PASS_DEPENDENCY(TargetTransformInfoWrapperPass)
INITIALIZE_PASS_END(ExpandReductions, "expand-reductions",
                    "Expand reduction intrinsics", false, false)

FunctionPass *toolchain::createExpandReductionsPass() {
  return new ExpandReductions();
}

PreservedAnalyses ExpandReductionsPass::run(Function &F,
                                            FunctionAnalysisManager &AM) {
  const auto &TTI = AM.getResult<TargetIRAnalysis>(F);
  if (!expandReductions(F, &TTI))
    return PreservedAnalyses::all();
  PreservedAnalyses PA;
  PA.preserveSet<CFGAnalyses>();
  return PA;
}
