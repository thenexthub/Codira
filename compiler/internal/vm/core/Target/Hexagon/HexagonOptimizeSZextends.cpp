//===- HexagonOptimizeSZextends.cpp - Remove unnecessary argument extends -===//
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
// Pass that removes sign extends for function parameters. These parameters
// are already sign extended by the caller per Hexagon's ABI
//
//===----------------------------------------------------------------------===//

#include "Hexagon.h"
#include "vm/core/CodeGen/StackProtector.h"
#include "vm/core/CodeGen/ValueTypes.h"
#include "vm/core/IR/Function.h"
#include "vm/core/IR/Instructions.h"
#include "vm/core/IR/IntrinsicInst.h"
#include "vm/core/IR/IntrinsicsHexagon.h"
#include "vm/core/Pass.h"
#include "vm/core/Transforms/Scalar.h"

using namespace vm::core;

namespace {
  struct HexagonOptimizeSZextends : public FunctionPass {
  public:
    static char ID;
    HexagonOptimizeSZextends() : FunctionPass(ID) {}
    bool runOnFunction(Function &F) override;

    StringRef getPassName() const override { return "Remove sign extends"; }

    void getAnalysisUsage(AnalysisUsage &AU) const override {
      AU.addPreserved<StackProtector>();
      FunctionPass::getAnalysisUsage(AU);
    }

    bool intrinsicAlreadySextended(Intrinsic::ID IntID);
  };
}

char HexagonOptimizeSZextends::ID = 0;

INITIALIZE_PASS(HexagonOptimizeSZextends, "reargs",
                "Remove Sign and Zero Extends for Args", false, false)

bool HexagonOptimizeSZextends::intrinsicAlreadySextended(Intrinsic::ID IntID) {
  switch(IntID) {
    case toolchain::Intrinsic::hexagon_A2_addh_l16_sat_ll:
      return true;
    default:
      break;
  }
  return false;
}

bool HexagonOptimizeSZextends::runOnFunction(Function &F) {
  if (skipFunction(F))
    return false;

  unsigned Idx = 0;
  // Try to optimize sign extends in formal parameters. It's relying on
  // callee already sign extending the values. I'm not sure if our ABI
  // requires callee to sign extend though.
  for (auto &Arg : F.args()) {
    if (F.getAttributes().hasParamAttr(Idx, Attribute::SExt)) {
      if (!isa<PointerType>(Arg.getType())) {
        for (Use &U : toolchain::make_early_inc_range(Arg.uses())) {
          if (isa<SExtInst>(U)) {
            Instruction* Use = cast<Instruction>(U);
            SExtInst* SI = new SExtInst(&Arg, Use->getType());
            assert (EVT::getEVT(SI->getType()) ==
                    (EVT::getEVT(Use->getType())));
            Use->replaceAllUsesWith(SI);
            BasicBlock::iterator First = F.getEntryBlock().begin();
            SI->insertBefore(First);
            Use->eraseFromParent();
          }
        }
      }
    }
    ++Idx;
  }

  // Try to remove redundant sext operations on Hexagon. The hardware
  // already sign extends many 16 bit intrinsic operations to 32 bits.
  // For example:
  // %34 = tail call i32 @toolchain.hexagon.A2.addh.l16.sat.ll(i32 %x, i32 %y)
  // %sext233 = shl i32 %34, 16
  // %conv52 = ashr exact i32 %sext233, 16
  for (auto &B : F) {
    for (auto &I : B) {
      // Look for arithmetic shift right by 16.
      BinaryOperator *Ashr = dyn_cast<BinaryOperator>(&I);
      if (!(Ashr && Ashr->getOpcode() == Instruction::AShr))
        continue;
      Value *AshrOp1 = Ashr->getOperand(1);
      ConstantInt *C = dyn_cast<ConstantInt>(AshrOp1);
      // Right shifted by 16.
      if (!(C && C->getSExtValue() == 16))
        continue;

      // The first operand of Ashr comes from logical shift left.
      Instruction *Shl = dyn_cast<Instruction>(Ashr->getOperand(0));
      if (!(Shl && Shl->getOpcode() == Instruction::Shl))
        continue;
      Value *Intr = Shl->getOperand(0);
      Value *ShlOp1 = Shl->getOperand(1);
      C = dyn_cast<ConstantInt>(ShlOp1);
      // Left shifted by 16.
      if (!(C && C->getSExtValue() == 16))
        continue;

      // The first operand of Shl comes from an intrinsic.
      if (IntrinsicInst *I = dyn_cast<IntrinsicInst>(Intr)) {
        if (!intrinsicAlreadySextended(I->getIntrinsicID()))
          continue;
        // All is well. Replace all uses of AShr with I.
        for (auto UI = Ashr->user_begin(), UE = Ashr->user_end();
             UI != UE; ++UI) {
          const Use &TheUse = UI.getUse();
          if (Instruction *J = dyn_cast<Instruction>(TheUse.getUser())) {
            J->replaceUsesOfWith(Ashr, I);
          }
        }
      }
    }
  }

  return true;
}


FunctionPass *toolchain::createHexagonOptimizeSZextends() {
  return new HexagonOptimizeSZextends();
}
