//===- LazyBranchProbabilityInfo.cpp - Lazy Branch Probability Analysis ---===//
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
// This is an alternative analysis pass to BranchProbabilityInfoWrapperPass.
// The difference is that with this pass the branch probabilities are not
// computed when the analysis pass is executed but rather when the BPI results
// is explicitly requested by the analysis client.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Analysis/LazyBranchProbabilityInfo.h"
#include "vm/core/Analysis/LoopInfo.h"
#include "vm/core/Analysis/TargetLibraryInfo.h"
#include "vm/core/IR/Dominators.h"
#include "vm/core/InitializePasses.h"

using namespace vm::core;

#define DEBUG_TYPE "lazy-branch-prob"

INITIALIZE_PASS_BEGIN(LazyBranchProbabilityInfoPass, DEBUG_TYPE,
                      "Lazy Branch Probability Analysis", true, true)
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_END(LazyBranchProbabilityInfoPass, DEBUG_TYPE,
                    "Lazy Branch Probability Analysis", true, true)

char LazyBranchProbabilityInfoPass::ID = 0;

LazyBranchProbabilityInfoPass::LazyBranchProbabilityInfoPass()
    : FunctionPass(ID) {
  initializeLazyBranchProbabilityInfoPassPass(*PassRegistry::getPassRegistry());
}

void LazyBranchProbabilityInfoPass::print(raw_ostream &OS,
                                          const Module *) const {
  LBPI->getCalculated().print(OS);
}

void LazyBranchProbabilityInfoPass::getAnalysisUsage(AnalysisUsage &AU) const {
  // We require DT so it's available when LI is available. The LI updating code
  // asserts that DT is also present so if we don't make sure that we have DT
  // here, that assert will trigger.
  AU.addRequiredTransitive<DominatorTreeWrapperPass>();
  AU.addRequiredTransitive<LoopInfoWrapperPass>();
  AU.addRequiredTransitive<TargetLibraryInfoWrapperPass>();
  AU.setPreservesAll();
}

void LazyBranchProbabilityInfoPass::releaseMemory() { LBPI.reset(); }

bool LazyBranchProbabilityInfoPass::runOnFunction(Function &F) {
  LoopInfo &LI = getAnalysis<LoopInfoWrapperPass>().getLoopInfo();
  TargetLibraryInfo &TLI =
      getAnalysis<TargetLibraryInfoWrapperPass>().getTLI(F);
  LBPI = std::make_unique<LazyBranchProbabilityInfo>(&F, &LI, &TLI);
  return false;
}

void LazyBranchProbabilityInfoPass::getLazyBPIAnalysisUsage(AnalysisUsage &AU) {
  AU.addRequiredTransitive<LazyBranchProbabilityInfoPass>();
  AU.addRequiredTransitive<LoopInfoWrapperPass>();
  AU.addRequiredTransitive<TargetLibraryInfoWrapperPass>();
}

void toolchain::initializeLazyBPIPassPass(PassRegistry &Registry) {
  INITIALIZE_PASS_DEPENDENCY(LazyBranchProbabilityInfoPass);
  INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass);
  INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass);
}
