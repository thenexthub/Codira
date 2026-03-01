//===- LoopTransformWarning.cpp -  ----------------------------------------===//
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
// Emit warnings if forced code transformations have not been performed.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Transforms/Scalar/WarnMissedTransforms.h"
#include "vm/core/Analysis/LoopInfo.h"
#include "vm/core/Analysis/OptimizationRemarkEmitter.h"
#include "vm/core/Transforms/Utils/LoopUtils.h"

using namespace vm::core;

#define DEBUG_TYPE "transform-warning"

/// Emit warnings for forced (i.e. user-defined) loop transformations which have
/// still not been performed.
static void warnAboutLeftoverTransformations(Loop *L,
                                             OptimizationRemarkEmitter *ORE) {
  if (hasUnrollTransformation(L) == TM_ForcedByUser) {
    LLVM_DEBUG(dbgs() << "Leftover unroll transformation\n");
    ORE->emit(
        DiagnosticInfoOptimizationFailure(DEBUG_TYPE,
                                          "FailedRequestedUnrolling",
                                          L->getStartLoc(), L->getHeader())
        << "loop not unrolled: the optimizer was unable to perform the "
           "requested transformation; the transformation might be disabled or "
           "specified as part of an unsupported transformation ordering");
  }

  if (hasUnrollAndJamTransformation(L) == TM_ForcedByUser) {
    LLVM_DEBUG(dbgs() << "Leftover unroll-and-jam transformation\n");
    ORE->emit(
        DiagnosticInfoOptimizationFailure(DEBUG_TYPE,
                                          "FailedRequestedUnrollAndJamming",
                                          L->getStartLoc(), L->getHeader())
        << "loop not unroll-and-jammed: the optimizer was unable to perform "
           "the requested transformation; the transformation might be disabled "
           "or specified as part of an unsupported transformation ordering");
  }

  if (hasVectorizeTransformation(L) == TM_ForcedByUser) {
    LLVM_DEBUG(dbgs() << "Leftover vectorization transformation\n");
    std::optional<ElementCount> VectorizeWidth =
        getOptionalElementCountLoopAttribute(L);
    std::optional<int> InterleaveCount =
        getOptionalIntLoopAttribute(L, "toolchain.loop.interleave.count");

    if (!VectorizeWidth || VectorizeWidth->isVector())
      ORE->emit(
          DiagnosticInfoOptimizationFailure(DEBUG_TYPE,
                                            "FailedRequestedVectorization",
                                            L->getStartLoc(), L->getHeader())
          << "loop not vectorized: the optimizer was unable to perform the "
             "requested transformation; the transformation might be disabled "
             "or specified as part of an unsupported transformation ordering");
    else if (InterleaveCount.value_or(0) != 1)
      ORE->emit(
          DiagnosticInfoOptimizationFailure(DEBUG_TYPE,
                                            "FailedRequestedInterleaving",
                                            L->getStartLoc(), L->getHeader())
          << "loop not interleaved: the optimizer was unable to perform the "
             "requested transformation; the transformation might be disabled "
             "or specified as part of an unsupported transformation ordering");
  }

  if (hasDistributeTransformation(L) == TM_ForcedByUser) {
    LLVM_DEBUG(dbgs() << "Leftover distribute transformation\n");
    ORE->emit(
        DiagnosticInfoOptimizationFailure(DEBUG_TYPE,
                                          "FailedRequestedDistribution",
                                          L->getStartLoc(), L->getHeader())
        << "loop not distributed: the optimizer was unable to perform the "
           "requested transformation; the transformation might be disabled or "
           "specified as part of an unsupported transformation ordering");
  }
}

static void warnAboutLeftoverTransformations(Function *F, LoopInfo *LI,
                                             OptimizationRemarkEmitter *ORE) {
  for (auto *L : LI->getLoopsInPreorder())
    warnAboutLeftoverTransformations(L, ORE);
}

// New pass manager boilerplate
PreservedAnalyses
WarnMissedTransformationsPass::run(Function &F, FunctionAnalysisManager &AM) {
  // Do not warn about not applied transformations if optimizations are
  // disabled.
  if (F.hasOptNone())
    return PreservedAnalyses::all();

  auto &ORE = AM.getResult<OptimizationRemarkEmitterAnalysis>(F);
  auto &LI = AM.getResult<LoopAnalysis>(F);

  warnAboutLeftoverTransformations(&F, &LI, &ORE);

  return PreservedAnalyses::all();
}
