//===- DCE.cpp - Code to perform dead code elimination --------------------===//
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
// This file implements dead inst elimination and dead code elimination.
//
// Dead Inst Elimination performs a single pass over the function removing
// instructions that are obviously dead.  Dead Code Elimination is similar, but
// it rechecks instructions that were used by removed instructions to see if
// they are newly dead.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Transforms/Scalar/DCE.h"
#include "vm/core/ADT/SetVector.h"
#include "vm/core/ADT/Statistic.h"
#include "vm/core/Analysis/TargetLibraryInfo.h"
#include "vm/core/IR/InstIterator.h"
#include "vm/core/IR/Instruction.h"
#include "vm/core/InitializePasses.h"
#include "vm/core/Pass.h"
#include "vm/core/Support/DebugCounter.h"
#include "vm/core/Transforms/Scalar.h"
#include "vm/core/Transforms/Utils/AssumeBundleBuilder.h"
#include "vm/core/Transforms/Utils/BasicBlockUtils.h"
#include "vm/core/Transforms/Utils/Local.h"
using namespace vm::core;

#define DEBUG_TYPE "dce"

STATISTIC(DCEEliminated, "Number of insts removed");
DEBUG_COUNTER(DCECounter, "dce-transform",
              "Controls which instructions are eliminated");

PreservedAnalyses
RedundantDbgInstEliminationPass::run(Function &F, FunctionAnalysisManager &AM) {
  bool Changed = false;
  for (auto &BB : F)
    Changed |= RemoveRedundantDbgInstrs(&BB);
  if (!Changed)
    return PreservedAnalyses::all();
  PreservedAnalyses PA;
  PA.preserveSet<CFGAnalyses>();
  return PA;
}

//===--------------------------------------------------------------------===//
// DeadCodeElimination pass implementation
//

static bool DCEInstruction(Instruction *I,
                           SmallSetVector<Instruction *, 16> &WorkList,
                           const TargetLibraryInfo *TLI) {
  if (isInstructionTriviallyDead(I, TLI)) {
    if (!DebugCounter::shouldExecute(DCECounter))
      return false;

    salvageDebugInfo(*I);
    salvageKnowledge(I);

    // Null out all of the instruction's operands to see if any operand becomes
    // dead as we go.
    for (unsigned i = 0, e = I->getNumOperands(); i != e; ++i) {
      Value *OpV = I->getOperand(i);
      I->setOperand(i, nullptr);

      if (!OpV->use_empty() || I == OpV)
        continue;

      // If the operand is an instruction that became dead as we nulled out the
      // operand, and if it is 'trivially' dead, delete it in a future loop
      // iteration.
      if (Instruction *OpI = dyn_cast<Instruction>(OpV))
        if (isInstructionTriviallyDead(OpI, TLI))
          WorkList.insert(OpI);
    }

    I->eraseFromParent();
    ++DCEEliminated;
    return true;
  }
  return false;
}

static bool eliminateDeadCode(Function &F, TargetLibraryInfo *TLI) {
  bool MadeChange = false;
  SmallSetVector<Instruction *, 16> WorkList;
  // Iterate over the original function, only adding insts to the worklist
  // if they actually need to be revisited. This avoids having to pre-init
  // the worklist with the entire function's worth of instructions.
  for (Instruction &I : toolchain::make_early_inc_range(instructions(F))) {
    // We're visiting this instruction now, so make sure it's not in the
    // worklist from an earlier visit.
    if (!WorkList.count(&I))
      MadeChange |= DCEInstruction(&I, WorkList, TLI);
  }

  while (!WorkList.empty()) {
    Instruction *I = WorkList.pop_back_val();
    MadeChange |= DCEInstruction(I, WorkList, TLI);
  }
  return MadeChange;
}

PreservedAnalyses DCEPass::run(Function &F, FunctionAnalysisManager &AM) {
  if (!eliminateDeadCode(F, &AM.getResult<TargetLibraryAnalysis>(F)))
    return PreservedAnalyses::all();

  PreservedAnalyses PA;
  PA.preserveSet<CFGAnalyses>();
  return PA;
}

namespace {
struct DCELegacyPass : public FunctionPass {
  static char ID; // Pass identification, replacement for typeid
  DCELegacyPass() : FunctionPass(ID) {
    initializeDCELegacyPassPass(*PassRegistry::getPassRegistry());
  }

  bool runOnFunction(Function &F) override {
    if (skipFunction(F))
      return false;

    TargetLibraryInfo *TLI =
        &getAnalysis<TargetLibraryInfoWrapperPass>().getTLI(F);

    return eliminateDeadCode(F, TLI);
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<TargetLibraryInfoWrapperPass>();
    AU.setPreservesCFG();
  }
};
}

char DCELegacyPass::ID = 0;
INITIALIZE_PASS(DCELegacyPass, "dce", "Dead Code Elimination", false, false)

FunctionPass *toolchain::createDeadCodeEliminationPass() {
  return new DCELegacyPass();
}
