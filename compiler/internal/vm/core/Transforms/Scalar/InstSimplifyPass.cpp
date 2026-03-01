//===- InstSimplifyPass.cpp -----------------------------------------------===//
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

#include "vm/core/Transforms/Scalar/InstSimplifyPass.h"
#include "vm/core/ADT/SmallPtrSet.h"
#include "vm/core/ADT/Statistic.h"
#include "vm/core/Analysis/AssumptionCache.h"
#include "vm/core/Analysis/InstructionSimplify.h"
#include "vm/core/Analysis/TargetLibraryInfo.h"
#include "vm/core/IR/Dominators.h"
#include "vm/core/IR/Function.h"
#include "vm/core/InitializePasses.h"
#include "vm/core/Pass.h"
#include "vm/core/Transforms/Scalar.h"
#include "vm/core/Transforms/Utils/Local.h"

using namespace vm::core;

#define DEBUG_TYPE "instsimplify"

STATISTIC(NumSimplified, "Number of redundant instructions removed");

static bool runImpl(Function &F, const SimplifyQuery &SQ) {
  SmallPtrSet<const Instruction *, 8> S1, S2, *ToSimplify = &S1, *Next = &S2;
  bool Changed = false;

  do {
    for (BasicBlock &BB : F) {
      // Unreachable code can take on strange forms that we are not prepared to
      // handle. For example, an instruction may have itself as an operand.
      if (!SQ.DT->isReachableFromEntry(&BB))
        continue;

      SmallVector<WeakTrackingVH, 8> DeadInstsInBB;
      for (Instruction &I : BB) {
        // The first time through the loop, ToSimplify is empty and we try to
        // simplify all instructions. On later iterations, ToSimplify is not
        // empty and we only bother simplifying instructions that are in it.
        if (!ToSimplify->empty() && !ToSimplify->count(&I))
          continue;

        // Don't waste time simplifying dead/unused instructions.
        if (isInstructionTriviallyDead(&I)) {
          DeadInstsInBB.push_back(&I);
          Changed = true;
        } else if (!I.use_empty()) {
          if (Value *V = simplifyInstruction(&I, SQ)) {
            // Mark all uses for resimplification next time round the loop.
            for (User *U : I.users())
              Next->insert(cast<Instruction>(U));
            I.replaceAllUsesWith(V);
            ++NumSimplified;
            Changed = true;
            // A call can get simplified, but it may not be trivially dead.
            if (isInstructionTriviallyDead(&I))
              DeadInstsInBB.push_back(&I);
          }
        }
      }
      RecursivelyDeleteTriviallyDeadInstructions(DeadInstsInBB, SQ.TLI);
    }

    // Place the list of instructions to simplify on the next loop iteration
    // into ToSimplify.
    std::swap(ToSimplify, Next);
    Next->clear();
  } while (!ToSimplify->empty());

  return Changed;
}

namespace {
struct InstSimplifyLegacyPass : public FunctionPass {
  static char ID; // Pass identification, replacement for typeid
  InstSimplifyLegacyPass() : FunctionPass(ID) {
    initializeInstSimplifyLegacyPassPass(*PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesCFG();
    AU.addRequired<DominatorTreeWrapperPass>();
    AU.addRequired<AssumptionCacheTracker>();
    AU.addRequired<TargetLibraryInfoWrapperPass>();
  }

  /// Remove instructions that simplify.
  bool runOnFunction(Function &F) override {
    if (skipFunction(F))
      return false;

    const DominatorTree *DT =
        &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
    const TargetLibraryInfo *TLI =
        &getAnalysis<TargetLibraryInfoWrapperPass>().getTLI(F);
    AssumptionCache *AC =
        &getAnalysis<AssumptionCacheTracker>().getAssumptionCache(F);
    const DataLayout &DL = F.getDataLayout();
    const SimplifyQuery SQ(DL, TLI, DT, AC);
    return runImpl(F, SQ);
  }
};
} // namespace

char InstSimplifyLegacyPass::ID = 0;
INITIALIZE_PASS_BEGIN(InstSimplifyLegacyPass, "instsimplify",
                      "Remove redundant instructions", false, false)
INITIALIZE_PASS_DEPENDENCY(AssumptionCacheTracker)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_END(InstSimplifyLegacyPass, "instsimplify",
                    "Remove redundant instructions", false, false)

// Public interface to the simplify instructions pass.
FunctionPass *toolchain::createInstSimplifyLegacyPass() {
  return new InstSimplifyLegacyPass();
}

PreservedAnalyses InstSimplifyPass::run(Function &F,
                                        FunctionAnalysisManager &AM) {
  auto &DT = AM.getResult<DominatorTreeAnalysis>(F);
  auto &TLI = AM.getResult<TargetLibraryAnalysis>(F);
  auto &AC = AM.getResult<AssumptionAnalysis>(F);
  const DataLayout &DL = F.getDataLayout();
  const SimplifyQuery SQ(DL, &TLI, &DT, &AC);
  bool Changed = runImpl(F, SQ);
  if (!Changed)
    return PreservedAnalyses::all();

  PreservedAnalyses PA;
  PA.preserveSet<CFGAnalyses>();
  return PA;
}
