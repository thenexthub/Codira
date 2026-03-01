//===- LoopRotation.cpp - Loop Rotation Pass ------------------------------===//
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
// This file implements Loop Rotation Pass.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Transforms/Scalar/LoopRotation.h"
#include "vm/core/Analysis/AssumptionCache.h"
#include "vm/core/Analysis/InstructionSimplify.h"
#include "vm/core/Analysis/LazyBlockFrequencyInfo.h"
#include "vm/core/Analysis/LoopInfo.h"
#include "vm/core/Analysis/LoopPass.h"
#include "vm/core/Analysis/MemorySSA.h"
#include "vm/core/Analysis/MemorySSAUpdater.h"
#include "vm/core/Analysis/ScalarEvolution.h"
#include "vm/core/Analysis/TargetTransformInfo.h"
#include "vm/core/Support/CommandLine.h"
#include "vm/core/Transforms/Scalar.h"
#include "vm/core/Transforms/Utils/LoopRotationUtils.h"
#include "vm/core/Transforms/Utils/LoopUtils.h"
#include <optional>
using namespace vm::core;

#define DEBUG_TYPE "loop-rotate"

static cl::opt<unsigned> DefaultRotationThreshold(
    "rotation-max-header-size", cl::init(16), cl::Hidden,
    cl::desc("The default maximum header size for automatic loop rotation"));

static cl::opt<bool> PrepareForLTOOption(
    "rotation-prepare-for-lto", cl::init(false), cl::Hidden,
    cl::desc("Run loop-rotation in the prepare-for-lto stage. This option "
             "should be used for testing only."));

LoopRotatePass::LoopRotatePass(bool EnableHeaderDuplication, bool PrepareForLTO)
    : EnableHeaderDuplication(EnableHeaderDuplication),
      PrepareForLTO(PrepareForLTO) {}

void LoopRotatePass::printPipeline(
    raw_ostream &OS, function_ref<StringRef(StringRef)> MapClassName2PassName) {
  static_cast<PassInfoMixin<LoopRotatePass> *>(this)->printPipeline(
      OS, MapClassName2PassName);
  OS << "<";
  if (!EnableHeaderDuplication)
    OS << "no-";
  OS << "header-duplication;";

  if (!PrepareForLTO)
    OS << "no-";
  OS << "prepare-for-lto";
  OS << ">";
}

PreservedAnalyses LoopRotatePass::run(Loop &L, LoopAnalysisManager &AM,
                                      LoopStandardAnalysisResults &AR,
                                      LPMUpdater &) {
  // Vectorization requires loop-rotation. Use default threshold for loops the
  // user explicitly marked for vectorization, even when header duplication is
  // disabled.
  int Threshold =
      (EnableHeaderDuplication && !L.getHeader()->getParent()->hasMinSize()) ||
              hasVectorizeTransformation(&L) == TM_ForcedByUser
          ? DefaultRotationThreshold
          : 0;
  const DataLayout &DL = L.getHeader()->getDataLayout();
  const SimplifyQuery SQ = getBestSimplifyQuery(AR, DL);

  std::optional<MemorySSAUpdater> MSSAU;
  if (AR.MSSA)
    MSSAU = MemorySSAUpdater(AR.MSSA);
  bool Changed = LoopRotation(&L, &AR.LI, &AR.TTI, &AR.AC, &AR.DT, &AR.SE,
                              MSSAU ? &*MSSAU : nullptr, SQ, false, Threshold,
                              false, PrepareForLTO || PrepareForLTOOption);

  if (!Changed)
    return PreservedAnalyses::all();

  if (AR.MSSA && VerifyMemorySSA)
    AR.MSSA->verifyMemorySSA();

  auto PA = getLoopPassPreservedAnalyses();
  if (AR.MSSA)
    PA.preserve<MemorySSAAnalysis>();
  return PA;
}
