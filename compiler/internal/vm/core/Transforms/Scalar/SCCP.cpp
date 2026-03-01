//===- SCCP.cpp - Sparse Conditional Constant Propagation -----------------===//
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
// This file implements sparse conditional constant propagation and merging:
//
// Specifically, this:
//   * Assumes values are constant unless proven otherwise
//   * Assumes BasicBlocks are dead unless proven otherwise
//   * Proves values to be constant, and replaces them with constants
//   * Proves conditional branches to be unconditional
//
//===----------------------------------------------------------------------===//

#include "vm/core/Transforms/Scalar/SCCP.h"
#include "vm/core/ADT/SmallPtrSet.h"
#include "vm/core/ADT/SmallVector.h"
#include "vm/core/ADT/Statistic.h"
#include "vm/core/Analysis/AssumptionCache.h"
#include "vm/core/Analysis/DomTreeUpdater.h"
#include "vm/core/Analysis/GlobalsModRef.h"
#include "vm/core/Analysis/TargetLibraryInfo.h"
#include "vm/core/Analysis/ValueLatticeUtils.h"
#include "vm/core/Analysis/ValueTracking.h"
#include "vm/core/IR/BasicBlock.h"
#include "vm/core/IR/DerivedTypes.h"
#include "vm/core/IR/Function.h"
#include "vm/core/IR/InstrTypes.h"
#include "vm/core/IR/Instruction.h"
#include "vm/core/IR/Instructions.h"
#include "vm/core/IR/IntrinsicInst.h"
#include "vm/core/IR/PassManager.h"
#include "vm/core/IR/Type.h"
#include "vm/core/IR/Value.h"
#include "vm/core/Pass.h"
#include "vm/core/Support/Debug.h"
#include "vm/core/Support/raw_ostream.h"
#include "vm/core/Transforms/Scalar.h"
#include "vm/core/Transforms/Utils/Local.h"
#include "vm/core/Transforms/Utils/SCCPSolver.h"

using namespace vm::core;

#define DEBUG_TYPE "sccp"

STATISTIC(NumInstRemoved, "Number of instructions removed");
STATISTIC(NumDeadBlocks , "Number of basic blocks unreachable");
STATISTIC(NumInstReplaced,
          "Number of instructions replaced with (simpler) instruction");

// runSCCP() - Run the Sparse Conditional Constant Propagation algorithm,
// and return true if the function was modified.
static bool runSCCP(Function &F, const DataLayout &DL,
                    const TargetLibraryInfo *TLI, DominatorTree &DT,
                    AssumptionCache &AC) {
  LLVM_DEBUG(dbgs() << "SCCP on function '" << F.getName() << "'\n");
  SCCPSolver Solver(
      DL, [TLI](Function &F) -> const TargetLibraryInfo & { return *TLI; },
      F.getContext());

  Solver.addPredicateInfo(F, DT, AC);

  // While we don't do any actual inter-procedural analysis, still track
  // return values so we can infer attributes.
  if (canTrackReturnsInterprocedurally(&F))
    Solver.addTrackedFunction(&F);

  // Mark the first block of the function as being executable.
  Solver.markBlockExecutable(&F.front());

  // Initialize arguments based on attributes.
  for (Argument &AI : F.args())
    Solver.trackValueOfArgument(&AI);

  // Solve for constants.
  bool ResolvedUndefs = true;
  while (ResolvedUndefs) {
    Solver.solve();
    LLVM_DEBUG(dbgs() << "RESOLVING UNDEFs\n");
    ResolvedUndefs = Solver.resolvedUndefsIn(F);
  }

  bool MadeChanges = false;

  // If we decided that there are basic blocks that are dead in this function,
  // delete their contents now.  Note that we cannot actually delete the blocks,
  // as we cannot modify the CFG of the function.

  SmallPtrSet<Value *, 32> InsertedValues;
  SmallVector<BasicBlock *, 8> BlocksToErase;
  for (BasicBlock &BB : F) {
    if (!Solver.isBlockExecutable(&BB)) {
      LLVM_DEBUG(dbgs() << "  BasicBlock Dead:" << BB);
      ++NumDeadBlocks;
      BlocksToErase.push_back(&BB);
      MadeChanges = true;
      continue;
    }

    MadeChanges |= Solver.simplifyInstsInBlock(BB, InsertedValues,
                                               NumInstRemoved, NumInstReplaced);
  }

  // Remove unreachable blocks and non-feasible edges.
  DomTreeUpdater DTU(DT, DomTreeUpdater::UpdateStrategy::Lazy);
  for (BasicBlock *DeadBB : BlocksToErase)
    NumInstRemoved += changeToUnreachable(&*DeadBB->getFirstNonPHIIt(),
                                          /*PreserveLCSSA=*/false, &DTU);

  BasicBlock *NewUnreachableBB = nullptr;
  for (BasicBlock &BB : F)
    MadeChanges |= Solver.removeNonFeasibleEdges(&BB, DTU, NewUnreachableBB);

  for (BasicBlock *DeadBB : BlocksToErase)
    if (!DeadBB->hasAddressTaken())
      DTU.deleteBB(DeadBB);

  Solver.removeSSACopies(F);

  Solver.inferReturnAttributes();

  return MadeChanges;
}

PreservedAnalyses SCCPPass::run(Function &F, FunctionAnalysisManager &AM) {
  const DataLayout &DL = F.getDataLayout();
  auto &TLI = AM.getResult<TargetLibraryAnalysis>(F);
  auto &DT = AM.getResult<DominatorTreeAnalysis>(F);
  auto &AC = AM.getResult<AssumptionAnalysis>(F);
  if (!runSCCP(F, DL, &TLI, DT, AC))
    return PreservedAnalyses::all();

  auto PA = PreservedAnalyses();
  PA.preserve<DominatorTreeAnalysis>();
  return PA;
}
