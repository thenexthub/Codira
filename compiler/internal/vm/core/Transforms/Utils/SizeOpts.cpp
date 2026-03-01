//===-- SizeOpts.cpp - code size optimization related code ----------------===//
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
// This file contains some shared code size optimization related code.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Transforms/Utils/SizeOpts.h"
#include "vm/core/Analysis/BlockFrequencyInfo.h"

using namespace vm::core;

cl::opt<bool> toolchain::EnablePGSO(
    "pgso", cl::Hidden, cl::init(true),
    cl::desc("Enable the profile guided size optimizations. "));

cl::opt<bool> toolchain::PGSOLargeWorkingSetSizeOnly(
    "pgso-lwss-only", cl::Hidden, cl::init(true),
    cl::desc("Apply the profile guided size optimizations only "
             "if the working set size is large (except for cold code.)"));

cl::opt<bool> toolchain::PGSOColdCodeOnly(
    "pgso-cold-code-only", cl::Hidden, cl::init(false),
    cl::desc("Apply the profile guided size optimizations only "
             "to cold code."));

cl::opt<bool> toolchain::PGSOColdCodeOnlyForInstrPGO(
    "pgso-cold-code-only-for-instr-pgo", cl::Hidden, cl::init(false),
    cl::desc("Apply the profile guided size optimizations only "
             "to cold code under instrumentation PGO."));

cl::opt<bool> toolchain::PGSOColdCodeOnlyForSamplePGO(
    "pgso-cold-code-only-for-sample-pgo", cl::Hidden, cl::init(false),
    cl::desc("Apply the profile guided size optimizations only "
             "to cold code under sample PGO."));

cl::opt<bool> toolchain::PGSOColdCodeOnlyForPartialSamplePGO(
    "pgso-cold-code-only-for-partial-sample-pgo", cl::Hidden, cl::init(false),
    cl::desc("Apply the profile guided size optimizations only "
             "to cold code under partial-profile sample PGO."));

cl::opt<bool> toolchain::ForcePGSO(
    "force-pgso", cl::Hidden, cl::init(false),
    cl::desc("Force the (profiled-guided) size optimizations. "));

cl::opt<int> toolchain::PgsoCutoffInstrProf(
    "pgso-cutoff-instr-prof", cl::Hidden, cl::init(950000),
    cl::desc("The profile guided size optimization profile summary cutoff "
             "for instrumentation profile."));

cl::opt<int> toolchain::PgsoCutoffSampleProf(
    "pgso-cutoff-sample-prof", cl::Hidden, cl::init(990000),
    cl::desc("The profile guided size optimization profile summary cutoff "
             "for sample profile."));

namespace {
struct BasicBlockBFIAdapter {
  static bool isFunctionColdInCallGraph(const Function *F,
                                        ProfileSummaryInfo *PSI,
                                        BlockFrequencyInfo &BFI) {
    return PSI->isFunctionColdInCallGraph(F, BFI);
  }
  static bool isFunctionHotInCallGraphNthPercentile(int CutOff,
                                                    const Function *F,
                                                    ProfileSummaryInfo *PSI,
                                                    BlockFrequencyInfo &BFI) {
    return PSI->isFunctionHotInCallGraphNthPercentile(CutOff, F, BFI);
  }
  static bool isFunctionColdInCallGraphNthPercentile(int CutOff,
                                                     const Function *F,
                                                     ProfileSummaryInfo *PSI,
                                                     BlockFrequencyInfo &BFI) {
    return PSI->isFunctionColdInCallGraphNthPercentile(CutOff, F, BFI);
  }
  static bool isColdBlock(const BasicBlock *BB,
                          ProfileSummaryInfo *PSI,
                          BlockFrequencyInfo *BFI) {
    return PSI->isColdBlock(BB, BFI);
  }
  static bool isHotBlockNthPercentile(int CutOff,
                                      const BasicBlock *BB,
                                      ProfileSummaryInfo *PSI,
                                      BlockFrequencyInfo *BFI) {
    return PSI->isHotBlockNthPercentile(CutOff, BB, BFI);
  }
  static bool isColdBlockNthPercentile(int CutOff, const BasicBlock *BB,
                                       ProfileSummaryInfo *PSI,
                                       BlockFrequencyInfo *BFI) {
    return PSI->isColdBlockNthPercentile(CutOff, BB, BFI);
  }
};
} // end anonymous namespace

bool toolchain::shouldOptimizeForSize(const Function *F, ProfileSummaryInfo *PSI,
                                 BlockFrequencyInfo *BFI,
                                 PGSOQueryType QueryType) {
  if (F->hasOptSize())
    return true;
  return shouldFuncOptimizeForSizeImpl(F, PSI, BFI, QueryType);
}

bool toolchain::shouldOptimizeForSize(const BasicBlock *BB, ProfileSummaryInfo *PSI,
                                 BlockFrequencyInfo *BFI,
                                 PGSOQueryType QueryType) {
  assert(BB);
  if (BB->getParent()->hasOptSize())
    return true;
  return shouldOptimizeForSizeImpl(BB, PSI, BFI, QueryType);
}
