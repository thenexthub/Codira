//===--- GlobalLoopARCSequenceDataflow.cpp --------------------------------===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//

//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "arc-sequence-opts"
#include "GlobalLoopARCSequenceDataflow.h"
#include "ARCRegionState.h"
#include "RCStateTransitionVisitors.h"
#include "language/SILOptimizer/Analysis/ARCAnalysis.h"
#include "language/SILOptimizer/Analysis/PostOrderAnalysis.h"
#include "language/SILOptimizer/Analysis/RCIdentityAnalysis.h"
#include "language/Basic/Assertions.h"
#include "language/SIL/SILInstruction.h"
#include "language/SIL/SILFunction.h"
#include "language/SIL/SILSuccessor.h"
#include "language/SIL/CFG.h"
#include "language/SIL/SILModule.h"
#include "toolchain/ADT/PostOrderIterator.h"
#include "toolchain/ADT/Statistic.h"
#include "toolchain/Support/Debug.h"

using namespace language;

//===----------------------------------------------------------------------===//
//                                  Utility
//===----------------------------------------------------------------------===//

/// Returns true if it is defined to perform a bottom up from \p Succ to \p
/// Pred.
///
/// This is interesting because in such cases, we must pessimistically assume
/// that we are merging in the empty set from Succ into Pred or vis-a-versa.
static bool isDefinedMerge(const LoopRegion *Succ, const LoopRegion *Pred) {
  // If the predecessor region is an unknown control flow edge tail, the
  // dataflow that enters into the region bottom up is undefined in our model.
  if (Pred->isUnknownControlFlowEdgeTail())
    return false;

  // If the successor region is an unknown control flow edge head, the dataflow
  // that leaves the region bottom up is considered to be undefined in our
  // model.
  if (Succ->isUnknownControlFlowEdgeHead())
    return false;

  // Otherwise it is defined to perform the merge.
  return true;
}

//===----------------------------------------------------------------------===//
//                             Top Down Dataflow
//===----------------------------------------------------------------------===//

void LoopARCSequenceDataflowEvaluator::mergePredecessors(
    const LoopRegion *Region, ARCRegionState &State) {
  bool HasAtLeastOnePred = false;

  for (unsigned PredID : Region->getPreds()) {
    auto *PredRegion = LRFI->getRegion(PredID);
    auto &PredState = getARCState(PredRegion);

    TOOLCHAIN_DEBUG(toolchain::dbgs() << "    Merging Pred: " << PredID << "\n");

    // If this merge is undefined due to unknown control flow, assume that the
    // empty set is flowing into this block so clear all state and exit early.
    if (!isDefinedMerge(Region, PredRegion)) {
      State.clear();
      break;
    }

    if (HasAtLeastOnePred) {
      State.mergePredTopDown(PredState);
      continue;
    }

    State.initPredTopDown(PredState);
    HasAtLeastOnePred = true;
  }
}

bool LoopARCSequenceDataflowEvaluator::processLoopTopDown(const LoopRegion *R) {
  assert(!R->isBlock() && "Expecting to process a non-block region");
  TOOLCHAIN_DEBUG(toolchain::dbgs() << "Processing Loop#: " << R->getID() << "\n");

  bool NestingDetected = false;

  // For each RegionID in our reverse post order...
  for (unsigned SubregionIndex : R->getSubregions()) {
    auto *Subregion = LRFI->getRegion(SubregionIndex);
    auto &SubregionData = getARCState(Subregion);

    // This will always succeed since we have an entry for each BB in our RPOT.
    TOOLCHAIN_DEBUG(toolchain::dbgs() << "Processing Subregion#: " << SubregionIndex
                            << "\n");

    // Ignore blocks that allow leaks.
    if (SubregionData.allowsLeaks()) {
      TOOLCHAIN_DEBUG(toolchain::dbgs() << "Skipping leaking BB.\n");
      continue;
    }

    TOOLCHAIN_DEBUG(toolchain::dbgs() << "Merging Predecessors for subregion!\n");
    mergePredecessors(Subregion, SubregionData);

    // Then perform the dataflow.
    NestingDetected |= SubregionData.processTopDown(
        AA, RCFI, LRFI, UnmatchedRefCountInsts, DecToIncStateMap,
        RegionStateInfo, SetFactory);
  }

  return NestingDetected;
}

//===----------------------------------------------------------------------===//
//                             Bottom Up Dataflow
//===----------------------------------------------------------------------===//

void LoopARCSequenceDataflowEvaluator::mergeSuccessors(const LoopRegion *Region,
                                                       ARCRegionState &State) {

  bool HasAtLeastOneSucc = false;

  for (unsigned SuccID : Region->getLocalSuccs()) {
    auto *SuccRegion = LRFI->getRegion(SuccID);
    auto &SuccState = getARCState(SuccRegion);

    TOOLCHAIN_DEBUG(toolchain::dbgs() << "    Merging Local Succ: " << SuccID << "\n");

    // If this merge is undefined due to unknown control flow, assume that the
    // empty set is flowing into this block so clear all state and exit early.
    if (!isDefinedMerge(SuccRegion, Region)) {
      State.clear();
      break;
    }

    // If this successor allows for leaks, skip it. This can only happen at the
    // function level scope. Otherwise, the block with the unreachable
    // terminator will be a non-local successor.
    //
    // At some point we will expand this to check for regions that are post
    // dominated by unreachables.
    if (SuccState.allowsLeaks())
      continue;

    if (HasAtLeastOneSucc) {
      State.mergeSuccBottomUp(SuccState);
      continue;
    }

    State.initSuccBottomUp(SuccState);
    HasAtLeastOneSucc = true;
  }

  for (unsigned SuccID : Region->getNonLocalSuccs()) {
    auto *SuccRegion = LRFI->getRegionForNonLocalSuccessor(Region, SuccID);
    auto &SuccState = getARCState(SuccRegion);

    TOOLCHAIN_DEBUG(toolchain::dbgs() << "    Merging Non Local Succs: " << SuccID
                            << "\n");

    // Check if this block is post dominated by ARC unreachable
    // blocks. Otherwise we clear all state.
    //
    // TODO: We just check the block itself for now.
    if (SuccState.allowsLeaks()) {
      TOOLCHAIN_DEBUG(toolchain::dbgs() << "        Allows leaks skipping\n");
      continue;
    }

    // Otherwise, we treat it as unknown control flow.
    TOOLCHAIN_DEBUG(toolchain::dbgs() << "        Clearing state b/c of early exit\n");
    State.clear();
    break;
  }
}

/// Analyze a single BB for refcount inc/dec instructions.
///
/// If anything was found it will be added to DecToIncStateMap.
///
/// NestingDetected will be set to indicate that the block needs to be
/// reanalyzed if code motion occurs.
///
/// An epilogue release is a release that post dominates all other uses of a
/// pointer in a function that implies that the pointer is alive up to that
/// point. We "freeze" (i.e. do not attempt to remove or move) such releases if
/// FreezeOwnedArgEpilogueReleases is set. This is useful since in certain cases
/// due to dataflow issues, we cannot properly propagate the last use
/// information. Instead we run an extra iteration of the ARC optimizer with
/// this enabled in a side table so the information gets propagated everywhere in
/// the CFG.
bool LoopARCSequenceDataflowEvaluator::processLoopBottomUp(
    const LoopRegion *R, bool FreezeOwnedArgEpilogueReleases) {

  bool NestingDetected = false;

  // For each BB in our post order...
  for (unsigned SubregionIndex : R->getReverseSubregions()) {
    auto *Subregion = LRFI->getRegion(SubregionIndex);
    auto &SubregionData = getARCState(Subregion);

    // This will always succeed since we have an entry for each BB in our post
    // order.
    TOOLCHAIN_DEBUG(toolchain::dbgs()
               << "Processing Subregion#: " << SubregionIndex << "\n");

    TOOLCHAIN_DEBUG(toolchain::dbgs() << "Merging Successors!\n");
    mergeSuccessors(Subregion, SubregionData);

    // Then perform the region optimization.
    NestingDetected |= SubregionData.processBottomUp(
        AA, RCFI, EAFI, LRFI, FreezeOwnedArgEpilogueReleases,
        UnmatchedRefCountInsts, IncToDecStateMap, RegionStateInfo,
        SetFactory);
  }

  return NestingDetected;
}

//===----------------------------------------------------------------------===//
//                 Top Level ARC Sequence Dataflow Evaluator
//===----------------------------------------------------------------------===//

LoopARCSequenceDataflowEvaluator::LoopARCSequenceDataflowEvaluator(
    SILFunction &F, AliasAnalysis *AA, LoopRegionFunctionInfo *LRFI,
    SILLoopInfo *SLI, RCIdentityFunctionInfo *RCFI,
    EpilogueARCFunctionInfo *EAFI,
    ProgramTerminationFunctionInfo *PTFI,
    BlotMapVector<SILInstruction *, TopDownRefCountState> &DecToIncStateMap,
    BlotMapVector<SILInstruction *, BottomUpRefCountState> &IncToDecStateMap)
    : Allocator(), SetFactory(Allocator), F(F), AA(AA), LRFI(LRFI), SLI(SLI),
      RCFI(RCFI), EAFI(EAFI), DecToIncStateMap(DecToIncStateMap),
      IncToDecStateMap(IncToDecStateMap) {
  for (auto *R : LRFI->getRegions()) {
    bool AllowsLeaks = false;
    if (R->isBlock())
      AllowsLeaks |= PTFI->isProgramTerminatingBlock(R->getBlock());
    RegionStateInfo[R] = new (Allocator) ARCRegionState(R, AllowsLeaks);
  }
}

LoopARCSequenceDataflowEvaluator::~LoopARCSequenceDataflowEvaluator() {
  for (auto P : RegionStateInfo) {
    P.second->~ARCRegionState();
  }
}

bool LoopARCSequenceDataflowEvaluator::runOnLoop(
    const LoopRegion *R, bool FreezeOwnedArgEpilogueReleases,
    bool RecomputePostDomReleases) {
  TOOLCHAIN_DEBUG(toolchain::dbgs() << "Run on region:\n");
  TOOLCHAIN_DEBUG(R->dump(true));
  bool NestingDetected = processLoopBottomUp(R, FreezeOwnedArgEpilogueReleases);
  NestingDetected |= processLoopTopDown(R);
  TOOLCHAIN_DEBUG(
      toolchain::dbgs() << "*** Bottom-Up and Top-Down analysis results ***\n");
  TOOLCHAIN_DEBUG(dumpDataflowResults());
  return NestingDetected;
}

void LoopARCSequenceDataflowEvaluator::dumpDataflowResults() {
  toolchain::dbgs() << "IncToDecStateMap:\n";
  for (auto it : IncToDecStateMap) {
    if (!it.has_value())
      continue;
    auto instAndState = it.value();
    toolchain::dbgs() << "Increment: ";
    instAndState.first->dump();
    instAndState.second.dump();
  }

  toolchain::dbgs() << "DecToIncStateMap:\n";
  for (auto it : DecToIncStateMap) {
    if (!it.has_value())
      continue;
    auto instAndState = it.value();
    toolchain::dbgs() << "Decrement: ";
    instAndState.first->dump();
    instAndState.second.dump();
  }
}

void LoopARCSequenceDataflowEvaluator::summarizeLoop(const LoopRegion *R) {
  RegionStateInfo[R]->summarize(LRFI, RegionStateInfo);
}

void LoopARCSequenceDataflowEvaluator::summarizeSubregionBlocks(
    const LoopRegion *R) {
  for (unsigned SubregionID : R->getSubregions()) {
    auto *Subregion = LRFI->getRegion(SubregionID);
    if (!Subregion->isBlock())
      continue;
    RegionStateInfo[Subregion]->summarizeBlock(Subregion->getBlock());
  }
}

void LoopARCSequenceDataflowEvaluator::clearLoopState(const LoopRegion *R) {
  for (unsigned SubregionID : R->getSubregions())
    getARCState(LRFI->getRegion(SubregionID)).clear();
}

void LoopARCSequenceDataflowEvaluator::addInterestingInst(SILInstruction *I) {
  auto *Region = LRFI->getRegion(I->getParent());
  RegionStateInfo[Region]->addInterestingInst(I);
}

void LoopARCSequenceDataflowEvaluator::removeInterestingInst(
    SILInstruction *I) {
  auto *Region = LRFI->getRegion(I->getParent());
  RegionStateInfo[Region]->removeInterestingInst(I);
}

// Compute if a RefCountInst was unmatched and populate in the persistent
// UnmatchedRefCountInsts.
// This can be done by looking up the RefCountInst in IncToDecStateMap or
// DecToIncStateMap. If the StrongIncrement was matched to a StrongDecrement,
// it will be found in IncToDecStateMap. If the StrongDecrement was matched to
// a StrongIncrement, it will be found in DecToIncStateMap.
void LoopARCSequenceDataflowEvaluator::saveMatchingInfo(const LoopRegion *R) {
  if (R->isFunction())
    return;
  for (unsigned SubregionID : R->getSubregions()) {
    auto *Subregion = LRFI->getRegion(SubregionID);
    if (!Subregion->isBlock())
      continue;
    auto *RegionState = RegionStateInfo[Subregion];
    for (auto Inst : RegionState->getSummarizedInterestingInsts()) {
      if (isa<StrongRetainInst>(Inst) || isa<RetainValueInst>(Inst)) {
        // unmatched if not found in IncToDecStateMap
        if (IncToDecStateMap.find(Inst) == IncToDecStateMap.end())
          UnmatchedRefCountInsts.insert(Inst);
        continue;
      }
      if (isa<StrongReleaseInst>(Inst) || isa<ReleaseValueInst>(Inst)) {
        // unmatched if not found in DecToIncStateMap
        if (DecToIncStateMap.find(Inst) == DecToIncStateMap.end())
          UnmatchedRefCountInsts.insert(Inst);
        continue;
      }
    }
  }
}
