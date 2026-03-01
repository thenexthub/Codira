//===-- MoveAutoInit.cpp - move auto-init inst closer to their use site----===//
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
// This pass moves instruction maked as auto-init closer to the basic block that
// use it, eventually removing it from some control path of the function.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Transforms/Utils/MoveAutoInit.h"
#include "vm/core/ADT/STLExtras.h"
#include "vm/core/ADT/Statistic.h"
#include "vm/core/Analysis/MemorySSA.h"
#include "vm/core/Analysis/MemorySSAUpdater.h"
#include "vm/core/Analysis/ValueTracking.h"
#include "vm/core/IR/DebugInfo.h"
#include "vm/core/IR/Dominators.h"
#include "vm/core/IR/Instructions.h"
#include "vm/core/IR/IntrinsicInst.h"
#include "vm/core/Support/CommandLine.h"
#include "vm/core/Transforms/Utils/LoopUtils.h"

using namespace vm::core;

#define DEBUG_TYPE "move-auto-init"

STATISTIC(NumMoved, "Number of instructions moved");

static cl::opt<unsigned> MoveAutoInitThreshold(
    "move-auto-init-threshold", cl::Hidden, cl::init(128),
    cl::desc("Maximum instructions to analyze per moved initialization"));

static bool hasAutoInitMetadata(const Instruction &I) {
  return I.hasMetadata(LLVMContext::MD_annotation) &&
         any_of(I.getMetadata(LLVMContext::MD_annotation)->operands(),
                [](const MDOperand &Op) { return Op.equalsStr("auto-init"); });
}

static std::optional<MemoryLocation> writeToAlloca(const Instruction &I) {
  MemoryLocation ML;
  if (auto *MI = dyn_cast<MemIntrinsic>(&I))
    ML = MemoryLocation::getForDest(MI);
  else if (auto *SI = dyn_cast<StoreInst>(&I))
    ML = MemoryLocation::get(SI);
  else
    return std::nullopt;

  if (isa<AllocaInst>(getUnderlyingObject(ML.Ptr)))
    return ML;
  else
    return {};
}

/// Finds a BasicBlock in the CFG where instruction `I` can be moved to while
/// not changing the Memory SSA ordering and being guarded by at least one
/// condition.
static BasicBlock *usersDominator(const MemoryLocation &ML, Instruction *I,
                                  DominatorTree &DT, MemorySSA &MSSA) {
  BasicBlock *CurrentDominator = nullptr;
  MemoryUseOrDef &IMA = *MSSA.getMemoryAccess(I);
  BatchAAResults AA(MSSA.getAA());

  SmallPtrSet<MemoryAccess *, 8> Visited;

  auto AsMemoryAccess = [](User *U) { return cast<MemoryAccess>(U); };
  SmallVector<MemoryAccess *> WorkList(map_range(IMA.users(), AsMemoryAccess));

  while (!WorkList.empty()) {
    MemoryAccess *MA = WorkList.pop_back_val();
    if (!Visited.insert(MA).second)
      continue;

    if (Visited.size() > MoveAutoInitThreshold)
      return nullptr;

    bool FoundClobberingUser = false;
    if (auto *M = dyn_cast<MemoryUseOrDef>(MA)) {
      Instruction *MI = M->getMemoryInst();

      // If this memory instruction may not clobber `I`, we can skip it.
      // LifetimeEnd is a valid user, but we do not want it in the user
      // dominator.
      if (AA.getModRefInfo(MI, ML) != ModRefInfo::NoModRef &&
          !MI->isLifetimeStartOrEnd() && MI != I) {
        FoundClobberingUser = true;
        CurrentDominator = CurrentDominator
                               ? DT.findNearestCommonDominator(CurrentDominator,
                                                               MI->getParent())
                               : MI->getParent();
      }
    }
    if (!FoundClobberingUser) {
      auto UsersAsMemoryAccesses = map_range(MA->users(), AsMemoryAccess);
      append_range(WorkList, UsersAsMemoryAccesses);
    }
  }
  return CurrentDominator;
}

static bool runMoveAutoInit(Function &F, DominatorTree &DT, MemorySSA &MSSA) {
  BasicBlock &EntryBB = F.getEntryBlock();
  SmallVector<std::pair<Instruction *, BasicBlock *>> JobList;

  //
  // Compute movable instructions.
  //
  for (Instruction &I : EntryBB) {
    if (!hasAutoInitMetadata(I))
      continue;

    std::optional<MemoryLocation> ML = writeToAlloca(I);
    if (!ML)
      continue;

    if (I.isVolatile())
      continue;

    BasicBlock *UsersDominator = usersDominator(ML.value(), &I, DT, MSSA);
    if (!UsersDominator)
      continue;

    if (UsersDominator == &EntryBB)
      continue;

    // Traverse the CFG to detect cycles `UsersDominator` would be part of.
    SmallPtrSet<BasicBlock *, 8> TransitiveSuccessors;
    SmallVector<BasicBlock *> WorkList(successors(UsersDominator));
    bool HasCycle = false;
    while (!WorkList.empty()) {
      BasicBlock *CurrBB = WorkList.pop_back_val();
      if (CurrBB == UsersDominator)
        // No early exit because we want to compute the full set of transitive
        // successors.
        HasCycle = true;
      for (BasicBlock *Successor : successors(CurrBB)) {
        if (!TransitiveSuccessors.insert(Successor).second)
          continue;
        WorkList.push_back(Successor);
      }
    }

    // Don't insert if that could create multiple execution of I,
    // but we can insert it in the non back-edge predecessors, if it exists.
    if (HasCycle) {
      BasicBlock *UsersDominatorHead = UsersDominator;
      while (BasicBlock *UniquePredecessor =
                 UsersDominatorHead->getUniquePredecessor())
        UsersDominatorHead = UniquePredecessor;

      if (UsersDominatorHead == &EntryBB)
        continue;

      BasicBlock *DominatingPredecessor = nullptr;
      for (BasicBlock *Pred : predecessors(UsersDominatorHead)) {
        // If one of the predecessor of the dominator also transitively is a
        // successor, moving to the dominator would do the inverse of loop
        // hoisting, and we don't want that.
        if (TransitiveSuccessors.count(Pred))
          continue;

        if (!DT.isReachableFromEntry(Pred))
          continue;

        DominatingPredecessor =
            DominatingPredecessor
                ? DT.findNearestCommonDominator(DominatingPredecessor, Pred)
                : Pred;
      }

      if (!DominatingPredecessor || DominatingPredecessor == &EntryBB)
        continue;

      UsersDominator = DominatingPredecessor;
    }

    // CatchSwitchInst blocks can only have one instruction, so they are not
    // good candidates for insertion.
    while (isa<CatchSwitchInst>(UsersDominator->getFirstNonPHIIt())) {
      for (BasicBlock *Pred : predecessors(UsersDominator))
        if (DT.isReachableFromEntry(Pred))
          UsersDominator = DT.findNearestCommonDominator(UsersDominator, Pred);
    }

    // We finally found a place where I can be moved while not introducing extra
    // execution, and guarded by at least one condition.
    if (UsersDominator != &EntryBB)
      JobList.emplace_back(&I, UsersDominator);
  }

  //
  // Perform the actual substitution.
  //
  if (JobList.empty())
    return false;

  MemorySSAUpdater MSSAU(&MSSA);

  // Reverse insertion to respect relative order between instructions:
  // if two instructions are moved from the same BB to the same BB, we insert
  // the second one in the front, then the first on top of it.
  for (auto &Job : reverse(JobList)) {
    Job.first->moveBefore(*Job.second, Job.second->getFirstInsertionPt());
    MSSAU.moveToPlace(MSSA.getMemoryAccess(Job.first), Job.first->getParent(),
                      MemorySSA::InsertionPlace::Beginning);
  }

  if (VerifyMemorySSA)
    MSSA.verifyMemorySSA();

  NumMoved += JobList.size();

  return true;
}

PreservedAnalyses MoveAutoInitPass::run(Function &F,
                                        FunctionAnalysisManager &AM) {

  auto &DT = AM.getResult<DominatorTreeAnalysis>(F);
  auto &MSSA = AM.getResult<MemorySSAAnalysis>(F).getMSSA();
  if (!runMoveAutoInit(F, DT, MSSA))
    return PreservedAnalyses::all();

  PreservedAnalyses PA;
  PA.preserve<DominatorTreeAnalysis>();
  PA.preserve<MemorySSAAnalysis>();
  PA.preserveSet<CFGAnalyses>();
  return PA;
}
