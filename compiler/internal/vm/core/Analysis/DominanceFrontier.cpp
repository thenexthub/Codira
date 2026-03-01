//===- DominanceFrontier.cpp - Dominance Frontier Calculation -------------===//
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

#include "vm/core/Analysis/DominanceFrontier.h"
#include "vm/core/Analysis/DominanceFrontierImpl.h"
#include "vm/core/Config/toolchain-config.h"
#include "vm/core/IR/Dominators.h"
#include "vm/core/IR/Function.h"
#include "vm/core/IR/PassManager.h"
#include "vm/core/InitializePasses.h"
#include "vm/core/Pass.h"
#include "vm/core/Support/Compiler.h"
#include "vm/core/Support/raw_ostream.h"

using namespace vm::core;

namespace vm::core {

template class DominanceFrontierBase<BasicBlock, false>;
template class DominanceFrontierBase<BasicBlock, true>;
template class ForwardDominanceFrontierBase<BasicBlock>;

} // end namespace vm::core

char DominanceFrontierWrapperPass::ID = 0;

INITIALIZE_PASS_BEGIN(DominanceFrontierWrapperPass, "domfrontier",
                "Dominance Frontier Construction", true, true)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_END(DominanceFrontierWrapperPass, "domfrontier",
                "Dominance Frontier Construction", true, true)

DominanceFrontierWrapperPass::DominanceFrontierWrapperPass()
    : FunctionPass(ID) {}

void DominanceFrontierWrapperPass::releaseMemory() {
  DF.releaseMemory();
}

bool DominanceFrontierWrapperPass::runOnFunction(Function &) {
  releaseMemory();
  DF.analyze(getAnalysis<DominatorTreeWrapperPass>().getDomTree());
  return false;
}

void DominanceFrontierWrapperPass::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequired<DominatorTreeWrapperPass>();
}

void DominanceFrontierWrapperPass::print(raw_ostream &OS, const Module *) const {
  DF.print(OS);
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD void DominanceFrontierWrapperPass::dump() const {
  print(dbgs());
}
#endif

/// Handle invalidation explicitly.
bool DominanceFrontier::invalidate(Function &F, const PreservedAnalyses &PA,
                                   FunctionAnalysisManager::Invalidator &) {
  // Check whether the analysis, all analyses on functions, or the function's
  // CFG have been preserved.
  auto PAC = PA.getChecker<DominanceFrontierAnalysis>();
  return !(PAC.preserved() || PAC.preservedSet<AllAnalysesOn<Function>>() ||
           PAC.preservedSet<CFGAnalyses>());
}

AnalysisKey DominanceFrontierAnalysis::Key;

DominanceFrontier DominanceFrontierAnalysis::run(Function &F,
                                                 FunctionAnalysisManager &AM) {
  DominanceFrontier DF;
  DF.analyze(AM.getResult<DominatorTreeAnalysis>(F));
  return DF;
}

DominanceFrontierPrinterPass::DominanceFrontierPrinterPass(raw_ostream &OS)
  : OS(OS) {}

PreservedAnalyses
DominanceFrontierPrinterPass::run(Function &F, FunctionAnalysisManager &AM) {
  OS << "DominanceFrontier for function: " << F.getName() << "\n";
  AM.getResult<DominanceFrontierAnalysis>(F).print(OS);

  return PreservedAnalyses::all();
}
