//===- LoopInstSimplify.cpp - Loop Instruction Simplification Pass --------===//
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
// This pass performs lightweight instruction simplification on loop bodies.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Transforms/Scalar/LoopInstSimplify.h"
#include "vm/core/ADT/STLExtras.h"
#include "vm/core/ADT/SmallPtrSet.h"
#include "vm/core/ADT/SmallVector.h"
#include "vm/core/ADT/Statistic.h"
#include "vm/core/Analysis/AssumptionCache.h"
#include "vm/core/Analysis/InstructionSimplify.h"
#include "vm/core/Analysis/LoopInfo.h"
#include "vm/core/Analysis/LoopIterator.h"
#include "vm/core/Analysis/LoopPass.h"
#include "vm/core/Analysis/MemorySSA.h"
#include "vm/core/Analysis/MemorySSAUpdater.h"
#include "vm/core/Analysis/TargetLibraryInfo.h"
#include "vm/core/IR/BasicBlock.h"
#include "vm/core/IR/Dominators.h"
#include "vm/core/IR/Instruction.h"
#include "vm/core/IR/Instructions.h"
#include "vm/core/Support/Casting.h"
#include "vm/core/Transforms/Utils/Local.h"
#include "vm/core/Transforms/Utils/LoopUtils.h"
#include <optional>
#include <utility>

using namespace vm::core;

#define DEBUG_TYPE "loop-instsimplify"

STATISTIC(NumSimplified, "Number of redundant instructions simplified");

static bool simplifyLoopInst(Loop &L, DominatorTree &DT, LoopInfo &LI,
                             AssumptionCache &AC, const TargetLibraryInfo &TLI,
                             MemorySSAUpdater *MSSAU) {
  const DataLayout &DL = L.getHeader()->getDataLayout();
  SimplifyQuery SQ(DL, &TLI, &DT, &AC);

  // On the first pass over the loop body we try to simplify every instruction.
  // On subsequent passes, we can restrict this to only simplifying instructions
  // where the inputs have been updated. We end up needing two sets: one
  // containing the instructions we are simplifying in *this* pass, and one for
  // the instructions we will want to simplify in the *next* pass. We use
  // pointers so we can swap between two stably allocated sets.
  SmallPtrSet<const Instruction *, 8> S1, S2, *ToSimplify = &S1, *Next = &S2;

  // Track the PHI nodes that have already been visited during each iteration so
  // that we can identify when it is necessary to iterate.
  SmallPtrSet<PHINode *, 4> VisitedPHIs;

  // While simplifying we may discover dead code or cause code to become dead.
  // Keep track of all such instructions and we will delete them at the end.
  SmallVector<WeakTrackingVH, 8> DeadInsts;

  // First we want to create an RPO traversal of the loop body. By processing in
  // RPO we can ensure that definitions are processed prior to uses (for non PHI
  // uses) in all cases. This ensures we maximize the simplifications in each
  // iteration over the loop and minimizes the possible causes for continuing to
  // iterate.
  LoopBlocksRPO RPOT(&L);
  RPOT.perform(&LI);
  MemorySSA *MSSA = MSSAU ? MSSAU->getMemorySSA() : nullptr;

  bool Changed = false;
  for (;;) {
    if (MSSAU && VerifyMemorySSA)
      MSSA->verifyMemorySSA();
    for (BasicBlock *BB : RPOT) {
      for (Instruction &I : *BB) {
        if (auto *PI = dyn_cast<PHINode>(&I))
          VisitedPHIs.insert(PI);

        if (I.use_empty()) {
          if (isInstructionTriviallyDead(&I, &TLI))
            DeadInsts.push_back(&I);
          continue;
        }

        // We special case the first iteration which we can detect due to the
        // empty `ToSimplify` set.
        bool IsFirstIteration = ToSimplify->empty();

        if (!IsFirstIteration && !ToSimplify->count(&I))
          continue;

        Value *V = simplifyInstruction(&I, SQ.getWithInstruction(&I));
        if (!V || !LI.replacementPreservesLCSSAForm(&I, V))
          continue;

        for (Use &U : toolchain::make_early_inc_range(I.uses())) {
          auto *UserI = cast<Instruction>(U.getUser());
          U.set(V);

          // Do not bother dealing with unreachable code.
          if (!DT.isReachableFromEntry(UserI->getParent()))
            continue;

          // If the instruction is used by a PHI node we have already processed
          // we'll need to iterate on the loop body to converge, so add it to
          // the next set.
          if (auto *UserPI = dyn_cast<PHINode>(UserI))
            if (VisitedPHIs.count(UserPI)) {
              Next->insert(UserPI);
              continue;
            }

          // If we are only simplifying targeted instructions and the user is an
          // instruction in the loop body, add it to our set of targeted
          // instructions. Because we process defs before uses (outside of PHIs)
          // we won't have visited it yet.
          //
          // We also skip any uses outside of the loop being simplified. Those
          // should always be PHI nodes due to LCSSA form, and we don't want to
          // try to simplify those away.
          assert((L.contains(UserI) || isa<PHINode>(UserI)) &&
                 "Uses outside the loop should be PHI nodes due to LCSSA!");
          if (!IsFirstIteration && L.contains(UserI))
            ToSimplify->insert(UserI);
        }

        if (MSSAU)
          if (Instruction *SimpleI = dyn_cast_or_null<Instruction>(V))
            if (MemoryAccess *MA = MSSA->getMemoryAccess(&I))
              if (MemoryAccess *ReplacementMA = MSSA->getMemoryAccess(SimpleI))
                MA->replaceAllUsesWith(ReplacementMA);

        assert(I.use_empty() && "Should always have replaced all uses!");
        if (isInstructionTriviallyDead(&I, &TLI))
          DeadInsts.push_back(&I);
        ++NumSimplified;
        Changed = true;
      }
    }

    // Delete any dead instructions found thus far now that we've finished an
    // iteration over all instructions in all the loop blocks.
    if (!DeadInsts.empty()) {
      Changed = true;
      RecursivelyDeleteTriviallyDeadInstructions(DeadInsts, &TLI, MSSAU);
    }

    if (MSSAU && VerifyMemorySSA)
      MSSA->verifyMemorySSA();

    // If we never found a PHI that needs to be simplified in the next
    // iteration, we're done.
    if (Next->empty())
      break;

    // Otherwise, put the next set in place for the next iteration and reset it
    // and the visited PHIs for that iteration.
    std::swap(Next, ToSimplify);
    Next->clear();
    VisitedPHIs.clear();
    DeadInsts.clear();
  }

  return Changed;
}

PreservedAnalyses LoopInstSimplifyPass::run(Loop &L, LoopAnalysisManager &AM,
                                            LoopStandardAnalysisResults &AR,
                                            LPMUpdater &) {
  std::optional<MemorySSAUpdater> MSSAU;
  if (AR.MSSA) {
    MSSAU = MemorySSAUpdater(AR.MSSA);
    if (VerifyMemorySSA)
      AR.MSSA->verifyMemorySSA();
  }
  if (!simplifyLoopInst(L, AR.DT, AR.LI, AR.AC, AR.TLI,
                        MSSAU ? &*MSSAU : nullptr))
    return PreservedAnalyses::all();

  auto PA = getLoopPassPreservedAnalyses();
  PA.preserveSet<CFGAnalyses>();
  if (AR.MSSA)
    PA.preserve<MemorySSAAnalysis>();
  return PA;
}
