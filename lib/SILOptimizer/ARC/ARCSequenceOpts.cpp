//===--- ARCSequenceOpts.cpp ----------------------------------------------===//
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

#include "ARCSequenceOpts.h"
#include "language/SIL/SILBuilder.h"
#include "language/SIL/SILVisitor.h"
#include "language/SILOptimizer/Analysis/ARCAnalysis.h"
#include "language/SILOptimizer/Analysis/AliasAnalysis.h"
#include "language/SILOptimizer/Analysis/EpilogueARCAnalysis.h"
#include "language/SILOptimizer/Analysis/LoopAnalysis.h"
#include "language/SILOptimizer/Analysis/LoopRegionAnalysis.h"
#include "language/SILOptimizer/Analysis/PostOrderAnalysis.h"
#include "language/SILOptimizer/Analysis/ProgramTerminationAnalysis.h"
#include "language/SILOptimizer/Analysis/RCIdentityAnalysis.h"
#include "language/SILOptimizer/PassManager/Passes.h"
#include "language/SILOptimizer/PassManager/Transforms.h"
#include "language/SILOptimizer/Utils/InstOptUtils.h"
#include "language/SILOptimizer/Utils/LoopUtils.h"
#include "toolchain/ADT/MapVector.h"
#include "toolchain/ADT/PointerUnion.h"
#include "toolchain/ADT/SmallPtrSet.h"
#include "toolchain/ADT/Statistic.h"
#include "toolchain/ADT/StringSwitch.h"
#include "toolchain/Support/CommandLine.h"
#include "toolchain/Support/Debug.h"

using namespace language;

STATISTIC(NumRefCountOpsRemoved, "Total number of increments removed");

toolchain::cl::opt<bool> EnableLoopARC("enable-loop-arc", toolchain::cl::init(true));

//===----------------------------------------------------------------------===//
//                                Code Motion
//===----------------------------------------------------------------------===//

// This routine takes in the ARCMatchingSet \p MatchSet and adds the increments
// and decrements to the delete list.
void ARCPairingContext::optimizeMatchingSet(
    ARCMatchingSet &MatchSet,
    toolchain::SmallVectorImpl<SILInstruction *> &DeadInsts) {
  TOOLCHAIN_DEBUG(toolchain::dbgs() << "**** Optimizing Matching Set ****\n");
  // Add the old increments to the delete list.
  for (SILInstruction *Increment : MatchSet.Increments) {
    MadeChange = true;
    TOOLCHAIN_DEBUG(toolchain::dbgs() << "    Deleting increment: " << *Increment);
    DeadInsts.push_back(Increment);
    ++NumRefCountOpsRemoved;
  }

  // Add the old decrements to the delete list.
  for (SILInstruction *Decrement : MatchSet.Decrements) {
    MadeChange = true;
    TOOLCHAIN_DEBUG(toolchain::dbgs() << "    Deleting decrement: " << *Decrement);
    DeadInsts.push_back(Decrement);
    ++NumRefCountOpsRemoved;
  }
}

bool ARCPairingContext::performMatching(
    toolchain::SmallVectorImpl<SILInstruction *> &DeadInsts) {
  bool MatchedPair = false;

  TOOLCHAIN_DEBUG(toolchain::dbgs() << "**** Computing ARC Matching Sets for "
                          << F.getName() << " ****\n");

  /// For each increment that we matched to a decrement, try to match it to a
  /// decrement -> increment pair.
  for (auto Pair : IncToDecStateMap) {
    if (!Pair.has_value())
      continue;

    SILInstruction *Increment = Pair->first;
    if (!Increment)
      continue; // blotted

    TOOLCHAIN_DEBUG(toolchain::dbgs() << "Constructing Matching Set For: " << *Increment);
    ARCMatchingSetBuilder Builder(DecToIncStateMap, IncToDecStateMap, RCIA);
    Builder.init(Increment);
    if (Builder.matchUpIncDecSetsForPtr()) {
      MatchedPair |= Builder.matchedPair();
      auto &Set = Builder.getResult();
      for (auto *I : Set.Increments)
        IncToDecStateMap.erase(I);
      for (auto *I : Set.Decrements)
        DecToIncStateMap.erase(I);

      optimizeMatchingSet(Set, DeadInsts);
    }
  }

  return MatchedPair;
}

//===----------------------------------------------------------------------===//
//                                  Loop ARC
//===----------------------------------------------------------------------===//

void LoopARCPairingContext::runOnLoop(SILLoop *L) {
  auto *Region = LRFI->getRegion(L);
  if (processRegion(Region, false, false)) {
    // We do not recompute for now since we only look at the top function level
    // for post dominating releases.
    processRegion(Region, true, false);
  }

  // Now that we have finished processing the loop, summarize the loop.
  Evaluator.summarizeLoop(Region);
}

void LoopARCPairingContext::runOnFunction(SILFunction *F) {
  if (processRegion(LRFI->getTopLevelRegion(), false, false)) {
    // We recompute the final post dom release since we may have moved the final
    // post dominated releases.
    processRegion(LRFI->getTopLevelRegion(), true, true);
  }
}

bool LoopARCPairingContext::processRegion(const LoopRegion *Region,
                                          bool FreezePostDomReleases,
                                          bool RecomputePostDomReleases) {
  toolchain::SmallVector<SILInstruction *, 8> DeadInsts;

  // We have already summarized all subloops of this loop. Now summarize our
  // blocks so that we only visit interesting instructions.
  Evaluator.summarizeSubregionBlocks(Region);

  bool MadeChange = false;
  bool NestingDetected = false;
  bool MatchedPair = false;

  do {
    NestingDetected = Evaluator.runOnLoop(Region, FreezePostDomReleases,
                                          RecomputePostDomReleases);
    MatchedPair = Context.performMatching(DeadInsts);

    if (!DeadInsts.empty()) {
      TOOLCHAIN_DEBUG(toolchain::dbgs() << "Removing dead interesting insts!\n");
      do {
        SILInstruction *I = DeadInsts.pop_back_val();
        TOOLCHAIN_DEBUG(toolchain::dbgs() << "    " << *I);
        Evaluator.removeInterestingInst(I);
        I->eraseFromParent();
      } while (!DeadInsts.empty());
    }

    MadeChange |= MatchedPair;
    Evaluator.saveMatchingInfo(Region);
    Evaluator.clearLoopState(Region);
    Context.DecToIncStateMap.clear();
    Context.IncToDecStateMap.clear();
    Evaluator.clearSetFactory();

    // This ensures we only ever recompute post dominating releases on the first
    // iteration.
    RecomputePostDomReleases = false;
  } while (NestingDetected && MatchedPair);

  return MadeChange;
}

//===----------------------------------------------------------------------===//
//                             Non Loop Optimizer
//===----------------------------------------------------------------------===//

static bool
processFunctionWithoutLoopSupport(SILFunction &F, bool FreezePostDomReleases,
                                  AliasAnalysis *AA, PostOrderAnalysis *POTA,
                                  RCIdentityFunctionInfo *RCIA,
                                  EpilogueARCFunctionInfo *EAFI,
                                  ProgramTerminationFunctionInfo *PTFI) {
  // GlobalARCOpts seems to be taking up a lot of compile time when running on
  // globalinit_func. Since that is not *that* interesting from an ARC
  // perspective (i.e. no ref count operations in a loop), disable it on such
  // functions temporarily in order to unblock others. This should be removed.
  if (F.isGlobalInitOnceFunction())
    return false;

  TOOLCHAIN_DEBUG(toolchain::dbgs() << "***** Processing " << F.getName() << " *****\n");

  bool Changed = false;
  BlockARCPairingContext Context(F, AA, POTA, RCIA, EAFI, PTFI);
  // Until we do not remove any instructions or have nested increments,
  // decrements...
  while (true) {
    // Compute matching sets of increments, decrements, and their insertion
    // points.
    //
    // We need to blot pointers we remove after processing an individual pointer
    // so we don't process pairs after we have paired them up. Thus we pass in a
    // lambda that performs the work for us.
    bool ShouldRunAgain = Context.run(FreezePostDomReleases);

    Changed |= Context.madeChange();

    // If we did not remove any instructions or have any nested increments, do
    // not perform another iteration.
    if (!ShouldRunAgain)
      break;

    // Otherwise, perform another iteration.
    TOOLCHAIN_DEBUG(toolchain::dbgs() << "\n<<< Made a Change! "
                               "Reprocessing Function! >>>\n");
  }

  TOOLCHAIN_DEBUG(toolchain::dbgs() << "\n");

  // Return true if we moved or deleted any instructions.
  return Changed;
}

//===----------------------------------------------------------------------===//
//                               Loop Optimizer
//===----------------------------------------------------------------------===//

static bool processFunctionWithLoopSupport(
    SILFunction &F, AliasAnalysis *AA, PostOrderAnalysis *POTA, 
    LoopRegionFunctionInfo *LRFI, SILLoopInfo *LI, RCIdentityFunctionInfo *RCFI,
    EpilogueARCFunctionInfo *EAFI, ProgramTerminationFunctionInfo *PTFI) {
  // GlobalARCOpts seems to be taking up a lot of compile time when running on
  // globalinit_func. Since that is not *that* interesting from an ARC
  // perspective (i.e. no ref count operations in a loop), disable it on such
  // functions temporarily in order to unblock others. This should be removed.
  if (F.isGlobalInitOnceFunction())
    return false;

  TOOLCHAIN_DEBUG(toolchain::dbgs() << "***** Processing " << F.getName() << " *****\n");

  LoopARCPairingContext Context(F, AA, LRFI, LI, RCFI, EAFI, PTFI);
  return Context.process();
}

//===----------------------------------------------------------------------===//
//                              Top Level Driver
//===----------------------------------------------------------------------===//

namespace {
class ARCSequenceOpts : public SILFunctionTransform {
  /// The entry point to the transformation.
  void run() override {
    auto *F = getFunction();

    // If ARC optimizations are disabled, don't optimize anything and bail.
    if (!getOptions().EnableARCOptimizations)
      return;

    // FIXME: We should support ownership.
    if (F->hasOwnership())
      return;

    if (!EnableLoopARC) {
      auto *AA = getAnalysis<AliasAnalysis>(F);
      auto *POTA = getAnalysis<PostOrderAnalysis>();
      auto *RCFI = getAnalysis<RCIdentityAnalysis>()->get(F);
      auto *EAFI = getAnalysis<EpilogueARCAnalysis>()->get(F);
      ProgramTerminationFunctionInfo PTFI(F);

      if (processFunctionWithoutLoopSupport(*F, false, AA, POTA, RCFI, EAFI, &PTFI)) {
        processFunctionWithoutLoopSupport(*F, true, AA, POTA, RCFI, EAFI, &PTFI);
        invalidateAnalysis(SILAnalysis::InvalidationKind::CallsAndInstructions);
      }
      return;
    }

    auto *LA = getAnalysis<SILLoopAnalysis>();
    auto *LI = LA->get(F);
    auto *DA = getAnalysis<DominanceAnalysis>();
    auto *DI = DA->get(F);

    // Canonicalize the loops, invalidating if we need to.
    if (canonicalizeAllLoops(DI, LI)) {
      // We preserve loop info and the dominator tree.
      DA->lockInvalidation();
      LA->lockInvalidation();
      PM->invalidateAnalysis(F, SILAnalysis::InvalidationKind::FunctionBody);
      DA->unlockInvalidation();
      LA->unlockInvalidation();
    }

    auto *AA = getAnalysis<AliasAnalysis>(F);
    auto *POTA = getAnalysis<PostOrderAnalysis>();
    auto *RCFI = getAnalysis<RCIdentityAnalysis>()->get(F);
    auto *EAFI = getAnalysis<EpilogueARCAnalysis>()->get(F);
    auto *LRFI = getAnalysis<LoopRegionAnalysis>()->get(F);
    ProgramTerminationFunctionInfo PTFI(F);

    if (processFunctionWithLoopSupport(*F, AA, POTA, LRFI, LI, RCFI, EAFI, &PTFI)) {
      invalidateAnalysis(SILAnalysis::InvalidationKind::CallsAndInstructions);
    }

  }
};

} // end anonymous namespace


SILTransform *language::createARCSequenceOpts() {
  return new ARCSequenceOpts();
}
