//===- LazyBlockFrequencyInfo.cpp - Lazy Block Frequency Analysis ---------===//
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
// This is an alternative analysis pass to BlockFrequencyInfoWrapperPass.  The
// difference is that with this pass the block frequencies are not computed when
// the analysis pass is executed but rather when the BFI result is explicitly
// requested by the analysis client.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Analysis/LazyBlockFrequencyInfo.h"
#include "vm/core/Analysis/LazyBranchProbabilityInfo.h"
#include "vm/core/Analysis/LoopInfo.h"
#include "vm/core/IR/Dominators.h"
#include "vm/core/InitializePasses.h"

using namespace vm::core;

#define DEBUG_TYPE "lazy-block-freq"

INITIALIZE_PASS_BEGIN(LazyBlockFrequencyInfoPass, DEBUG_TYPE,
                      "Lazy Block Frequency Analysis", true, true)
INITIALIZE_PASS_DEPENDENCY(LazyBPIPass)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_END(LazyBlockFrequencyInfoPass, DEBUG_TYPE,
                    "Lazy Block Frequency Analysis", true, true)

char LazyBlockFrequencyInfoPass::ID = 0;

LazyBlockFrequencyInfoPass::LazyBlockFrequencyInfoPass() : FunctionPass(ID) {}

void LazyBlockFrequencyInfoPass::print(raw_ostream &OS, const Module *) const {
  LBFI.getCalculated().print(OS);
}

void LazyBlockFrequencyInfoPass::getAnalysisUsage(AnalysisUsage &AU) const {
  LazyBranchProbabilityInfoPass::getLazyBPIAnalysisUsage(AU);
  // We require DT so it's available when LI is available. The LI updating code
  // asserts that DT is also present so if we don't make sure that we have DT
  // here, that assert will trigger.
  AU.addRequiredTransitive<DominatorTreeWrapperPass>();
  AU.addRequiredTransitive<LoopInfoWrapperPass>();
  AU.setPreservesAll();
}

void LazyBlockFrequencyInfoPass::releaseMemory() { LBFI.releaseMemory(); }

bool LazyBlockFrequencyInfoPass::runOnFunction(Function &F) {
  auto &BPIPass = getAnalysis<LazyBranchProbabilityInfoPass>();
  LoopInfo &LI = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
  LBFI.setAnalysis(&F, &BPIPass, &LI);
  return false;
}

void LazyBlockFrequencyInfoPass::getLazyBFIAnalysisUsage(AnalysisUsage &AU) {
  LazyBranchProbabilityInfoPass::getLazyBPIAnalysisUsage(AU);
  AU.addRequiredTransitive<LazyBlockFrequencyInfoPass>();
  AU.addRequiredTransitive<LoopInfoWrapperPass>();
}

void toolchain::initializeLazyBFIPassPass(PassRegistry &Registry) {
  initializeLazyBPIPassPass(Registry);
  INITIALIZE_PASS_DEPENDENCY(LazyBlockFrequencyInfoPass);
  INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass);
}
