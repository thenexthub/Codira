//===--- MisExpect.cpp - Check the use of toolchain.expect with PGO data -------===//
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
// This contains code to emit warnings for potentially incorrect usage of the
// toolchain.expect intrinsic. This utility extracts the threshold values from
// metadata associated with the instrumented Branch or Switch instruction. The
// threshold values are then used to determine if a warning should be emmited.
//
// MisExpect's implementation relies on two assumptions about how branch weights
// are managed in LLVM.
//
// 1) Frontend profiling weights are always in place before toolchain.expect is
// lowered in LowerExpectIntrinsic.cpp. Frontend based instrumentation therefore
// needs to extract the branch weights and then compare them to the weights
// being added by the toolchain.expect intrinsic lowering.
//
// 2) Sampling and IR based profiles will *only* have branch weight metadata
// before profiling data is consulted if they are from a lowered toolchain.expect
// intrinsic. These profiles thus always extract the expected weights and then
// compare them to the weights collected during profiling to determine if a
// diagnostic message is warranted.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Transforms/Utils/MisExpect.h"
#include "vm/core/ADT/Twine.h"
#include "vm/core/Analysis/OptimizationRemarkEmitter.h"
#include "vm/core/IR/DiagnosticInfo.h"
#include "vm/core/IR/Instruction.h"
#include "vm/core/IR/Instructions.h"
#include "vm/core/IR/LLVMContext.h"
#include "vm/core/IR/ProfDataUtils.h"
#include "vm/core/Support/BranchProbability.h"
#include "vm/core/Support/CommandLine.h"
#include "vm/core/Support/FormatVariadic.h"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <numeric>

#define DEBUG_TYPE "misexpect"

using namespace vm::core;
using namespace misexpect;

// Command line option to enable/disable the warning when profile data suggests
// a mismatch with the use of the toolchain.expect intrinsic
static cl::opt<bool> PGOWarnMisExpect(
    "pgo-warn-misexpect", cl::init(false), cl::Hidden,
    cl::desc("Use this option to turn on/off "
             "warnings about incorrect usage of toolchain.expect intrinsics."));

// Command line option for setting the diagnostic tolerance threshold
static cl::opt<uint32_t> MisExpectTolerance(
    "misexpect-tolerance", cl::init(0),
    cl::desc("Prevents emitting diagnostics when profile counts are "
             "within N% of the threshold.."));

static bool isMisExpectDiagEnabled(const LLVMContext &Ctx) {
  return PGOWarnMisExpect || Ctx.getMisExpectWarningRequested();
}

static uint32_t getMisExpectTolerance(const LLVMContext &Ctx) {
  return std::max(static_cast<uint32_t>(MisExpectTolerance),
                  Ctx.getDiagnosticsMisExpectTolerance());
}

static const Instruction *getInstCondition(const Instruction *I) {
  assert(I != nullptr && "MisExpect target Instruction cannot be nullptr");
  const Instruction *Ret = nullptr;
  if (auto *B = dyn_cast<BranchInst>(I)) {
    Ret = dyn_cast<Instruction>(B->getCondition());
  }
  // TODO: Find a way to resolve condition location for switches
  // Using the condition of the switch seems to often resolve to an earlier
  // point in the program, i.e. the calculation of the switch condition, rather
  // than the switch's location in the source code. Thus, we should use the
  // instruction to get source code locations rather than the condition to
  // improve diagnostic output, such as the caret. If the same problem exists
  // for branch instructions, then we should remove this function and directly
  // use the instruction
  //
  else if (auto *S = dyn_cast<SwitchInst>(I)) {
    Ret = dyn_cast<Instruction>(S->getCondition());
  }
  return Ret ? Ret : I;
}

static void emitMisexpectDiagnostic(const Instruction *I, LLVMContext &Ctx,
                                    uint64_t ProfCount, uint64_t TotalCount) {
  double PercentageCorrect = (double)ProfCount / TotalCount;
  auto PerString =
      formatv("{0:P} ({1} / {2})", PercentageCorrect, ProfCount, TotalCount);
  auto RemStr = formatv(
      "Potential performance regression from use of the toolchain.expect intrinsic: "
      "Annotation was correct on {0} of profiled executions.",
      PerString);
  const Instruction *Cond = getInstCondition(I);
  if (isMisExpectDiagEnabled(Ctx))
    Ctx.diagnose(DiagnosticInfoMisExpect(Cond, Twine(PerString)));
  OptimizationRemarkEmitter ORE(I->getParent()->getParent());
  ORE.emit(OptimizationRemark(DEBUG_TYPE, "misexpect", Cond) << RemStr.str());
}

void misexpect::verifyMisExpect(const Instruction &I,
                                ArrayRef<uint32_t> RealWeights,
                                ArrayRef<uint32_t> ExpectedWeights) {
  // To determine if we emit a diagnostic, we need to compare the branch weights
  // from the profile to those added by the toolchain.expect intrinsic.
  // So first, we extract the "likely" and "unlikely" weights from
  // ExpectedWeights And determine the correct weight in the profile to compare
  // against.
  uint64_t LikelyBranchWeight = 0,
           UnlikelyBranchWeight = std::numeric_limits<uint32_t>::max();
  size_t MaxIndex = 0;
  for (const auto &[Idx, V] : enumerate(ExpectedWeights)) {
    if (LikelyBranchWeight < V) {
      LikelyBranchWeight = V;
      MaxIndex = Idx;
    }
    if (UnlikelyBranchWeight > V)
      UnlikelyBranchWeight = V;
  }

  const uint64_t ProfiledWeight = RealWeights[MaxIndex];
  const uint64_t RealWeightsTotal =
      std::accumulate(RealWeights.begin(), RealWeights.end(), (uint64_t)0,
                      std::plus<uint64_t>());
  const uint64_t NumUnlikelyTargets = RealWeights.size() - 1;

  uint64_t TotalBranchWeight =
      LikelyBranchWeight + (UnlikelyBranchWeight * NumUnlikelyTargets);

  // Failing this assert means that we have corrupted metadata.
  assert((TotalBranchWeight >= LikelyBranchWeight) && (TotalBranchWeight > 0) &&
         "TotalBranchWeight is less than the Likely branch weight");

  // To determine our threshold value we need to obtain the branch probability
  // for the weights added by toolchain.expect and use that proportion to calculate
  // our threshold based on the collected profile data.
  auto LikelyProbablilty = BranchProbability::getBranchProbability(
      LikelyBranchWeight, TotalBranchWeight);

  uint64_t ScaledThreshold = LikelyProbablilty.scale(RealWeightsTotal);

  // clamp tolerance range to [0, 100)
  uint32_t Tolerance = getMisExpectTolerance(I.getContext());
  Tolerance = std::clamp(Tolerance, 0u, 99u);

  // Allow users to relax checking by N%  i.e., if they use a 5% tolerance,
  // then we check against 0.95*ScaledThreshold
  if (Tolerance > 0)
    ScaledThreshold *= (1.0 - Tolerance / 100.0);

  // When the profile weight is below the threshold, we emit the diagnostic
  if (ProfiledWeight < ScaledThreshold)
    emitMisexpectDiagnostic(&I, I.getContext(), ProfiledWeight,
                            RealWeightsTotal);
}

void misexpect::checkBackendInstrumentation(const Instruction &I,
                                            ArrayRef<uint32_t> RealWeights) {
  // Backend checking assumes any existing weight comes from an `toolchain.expect`
  // intrinsic. However, SampleProfiling + ThinLTO add branch weights  multiple
  // times, leading to an invalid assumption in our checking. Backend checks
  // should only operate on branch weights that carry the "!expected" field,
  // since they are guaranteed to be added by the LowerExpectIntrinsic pass.
  if (!hasBranchWeightOrigin(I))
    return;
  SmallVector<uint32_t> ExpectedWeights;
  if (!extractBranchWeights(I, ExpectedWeights))
    return;
  verifyMisExpect(I, RealWeights, ExpectedWeights);
}

void misexpect::checkFrontendInstrumentation(
    const Instruction &I, ArrayRef<uint32_t> ExpectedWeights) {
  SmallVector<uint32_t> RealWeights;
  if (!extractBranchWeights(I, RealWeights))
    return;
  verifyMisExpect(I, RealWeights, ExpectedWeights);
}

void misexpect::checkExpectAnnotations(const Instruction &I,
                                       ArrayRef<uint32_t> ExistingWeights,
                                       bool IsFrontend) {
  if (IsFrontend)
    checkFrontendInstrumentation(I, ExistingWeights);
  else
    checkBackendInstrumentation(I, ExistingWeights);
}
