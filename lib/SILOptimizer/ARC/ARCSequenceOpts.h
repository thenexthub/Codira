//===--- ARCSequenceOpts.h --------------------------------------*- C++ -*-===//
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

#ifndef LANGUAGE_SILOPTIMIZER_PASSMANAGER_ARCSEQUENCEOPTS_H
#define LANGUAGE_SILOPTIMIZER_PASSMANAGER_ARCSEQUENCEOPTS_H

#include "GlobalARCSequenceDataflow.h"
#include "GlobalLoopARCSequenceDataflow.h"
#include "ARCMatchingSet.h"
#include "language/SIL/SILValue.h"
#include "language/SILOptimizer/Utils/LoopUtils.h"
#include "toolchain/ADT/SetVector.h"

namespace language {

class SILInstruction;
class SILFunction;
class AliasAnalysis;
class PostOrderAnalysis;
class LoopRegionFunctionInfo;
class SILLoopInfo;
class RCIdentityFunctionInfo;

struct ARCPairingContext {
  SILFunction &F;
  BlotMapVector<SILInstruction *, TopDownRefCountState> DecToIncStateMap;
  BlotMapVector<SILInstruction *, BottomUpRefCountState> IncToDecStateMap;
  RCIdentityFunctionInfo *RCIA;
  bool MadeChange = false;

  ARCPairingContext(SILFunction &F, RCIdentityFunctionInfo *RCIA)
      : F(F), DecToIncStateMap(), IncToDecStateMap(), RCIA(RCIA) {}
  bool performMatching(toolchain::SmallVectorImpl<SILInstruction *> &DeadInsts);

  void optimizeMatchingSet(ARCMatchingSet &MatchSet,
                           toolchain::SmallVectorImpl<SILInstruction *> &DeadInsts);
};

/// A composition of an ARCSequenceDataflowEvaluator and an
/// ARCPairingContext. The evaluator performs top down/bottom up dataflows
/// clearing the dataflow at loop boundaries. Then the results of the evaluator
/// are placed into the ARCPairingContext and then the ARCPairingContext is used
/// to pair retains/releases.
struct BlockARCPairingContext {
  ARCPairingContext Context;
  ARCSequenceDataflowEvaluator Evaluator;

  BlockARCPairingContext(SILFunction &F, AliasAnalysis *AA,
                         PostOrderAnalysis *POTA, RCIdentityFunctionInfo *RCFI,
                         EpilogueARCFunctionInfo *EAFI,
                         ProgramTerminationFunctionInfo *PTFI)
      : Context(F, RCFI),
        Evaluator(F, AA, POTA, RCFI, EAFI, PTFI, Context.DecToIncStateMap,
                  Context.IncToDecStateMap) {}

  bool run(bool FreezePostDomReleases) {
    bool NestingDetected = Evaluator.run(FreezePostDomReleases);
    Evaluator.clear();

    toolchain::SmallVector<SILInstruction *, 8> DeadInsts;
    bool MatchedPair = Context.performMatching(DeadInsts);
    while (!DeadInsts.empty())
      DeadInsts.pop_back_val()->eraseFromParent();
    return NestingDetected && MatchedPair;
  }

  bool madeChange() const { return Context.MadeChange; }
};

/// A composition of a LoopARCSequenceDataflowEvaluator and an
/// ARCPairingContext. The loop nest is processed bottom up. For each loop, we
/// run the evaluator on the loop and then use the ARCPairingContext to pair
/// retains/releases and eliminate them.
struct LoopARCPairingContext : SILLoopVisitor {
  ARCPairingContext Context;
  LoopARCSequenceDataflowEvaluator Evaluator;
  LoopRegionFunctionInfo *LRFI;
  SILLoopInfo *SLI;

  LoopARCPairingContext(SILFunction &F, AliasAnalysis *AA,
                        LoopRegionFunctionInfo *LRFI, SILLoopInfo *SLI,
                        RCIdentityFunctionInfo *RCFI,
                        EpilogueARCFunctionInfo *EAFI,
                        ProgramTerminationFunctionInfo *PTFI)
      : SILLoopVisitor(&F, SLI), Context(F, RCFI),
        Evaluator(F, AA, LRFI, SLI, RCFI, EAFI, PTFI, Context.DecToIncStateMap,
                  Context.IncToDecStateMap),
        LRFI(LRFI), SLI(SLI) {}

  bool process() {
    run();
    if (!madeChange())
      return false;
    run();
    return true;
  }

  bool madeChange() const { return Context.MadeChange; }

  void runOnLoop(SILLoop *L) override;
  void runOnFunction(SILFunction *F) override;

  bool processRegion(const LoopRegion *R, bool FreezePostDomReleases,
                     bool RecomputePostDomReleases);
};

} // end language namespace

#endif
