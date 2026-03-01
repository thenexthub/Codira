//===- MachineDominators.cpp - Machine Dominator Calculation --------------===//
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
// This file implements simple dominator construction algorithms for finding
// forward dominators on machine functions.
//
//===----------------------------------------------------------------------===//

#include "vm/core/CodeGen/MachineDominators.h"
#include "vm/core/CodeGen/Passes.h"
#include "vm/core/InitializePasses.h"
#include "vm/core/Pass.h"
#include "vm/core/PassRegistry.h"
#include "vm/core/Support/CommandLine.h"
#include "vm/core/Support/Compiler.h"
#include "vm/core/Support/GenericDomTreeConstruction.h"

using namespace vm::core;

namespace vm::core {
// Always verify dominfo if expensive checking is enabled.
#ifdef EXPENSIVE_CHECKS
bool VerifyMachineDomInfo = true;
#else
bool VerifyMachineDomInfo = false;
#endif
} // namespace vm::core

static cl::opt<bool, true> VerifyMachineDomInfoX(
    "verify-machine-dom-info", cl::location(VerifyMachineDomInfo), cl::Hidden,
    cl::desc("Verify machine dominator info (time consuming)"));

namespace vm::core {
template class LLVM_EXPORT_TEMPLATE DomTreeNodeBase<MachineBasicBlock>;
template class LLVM_EXPORT_TEMPLATE
    DominatorTreeBase<MachineBasicBlock, false>; // DomTreeBase

namespace DomTreeBuilder {
template LLVM_EXPORT_TEMPLATE void Calculate<MBBDomTree>(MBBDomTree &DT);
template LLVM_EXPORT_TEMPLATE void
CalculateWithUpdates<MBBDomTree>(MBBDomTree &DT, MBBUpdates U);

template LLVM_EXPORT_TEMPLATE void
InsertEdge<MBBDomTree>(MBBDomTree &DT, MachineBasicBlock *From,
                       MachineBasicBlock *To);

template LLVM_EXPORT_TEMPLATE void
DeleteEdge<MBBDomTree>(MBBDomTree &DT, MachineBasicBlock *From,
                       MachineBasicBlock *To);

template LLVM_EXPORT_TEMPLATE void
ApplyUpdates<MBBDomTree>(MBBDomTree &DT, MBBDomTreeGraphDiff &,
                         MBBDomTreeGraphDiff *);

template LLVM_EXPORT_TEMPLATE bool
Verify<MBBDomTree>(const MBBDomTree &DT, MBBDomTree::VerificationLevel VL);
} // namespace DomTreeBuilder
}

bool MachineDominatorTree::invalidate(
    MachineFunction &, const PreservedAnalyses &PA,
    MachineFunctionAnalysisManager::Invalidator &) {
  // Check whether the analysis, all analyses on machine functions, or the
  // machine function's CFG have been preserved.
  auto PAC = PA.getChecker<MachineDominatorTreeAnalysis>();
  return !PAC.preserved() &&
         !PAC.preservedSet<AllAnalysesOn<MachineFunction>>() &&
         !PAC.preservedSet<CFGAnalyses>();
}

AnalysisKey MachineDominatorTreeAnalysis::Key;

MachineDominatorTreeAnalysis::Result
MachineDominatorTreeAnalysis::run(MachineFunction &MF,
                                  MachineFunctionAnalysisManager &) {
  return MachineDominatorTree(MF);
}

PreservedAnalyses
MachineDominatorTreePrinterPass::run(MachineFunction &MF,
                                     MachineFunctionAnalysisManager &MFAM) {
  OS << "MachineDominatorTree for machine function: " << MF.getName() << '\n';
  MFAM.getResult<MachineDominatorTreeAnalysis>(MF).print(OS);
  return PreservedAnalyses::all();
}

char MachineDominatorTreeWrapperPass::ID = 0;

INITIALIZE_PASS(MachineDominatorTreeWrapperPass, "machinedomtree",
                "MachineDominator Tree Construction", true, true)

MachineDominatorTreeWrapperPass::MachineDominatorTreeWrapperPass()
    : MachineFunctionPass(ID) {
  initializeMachineDominatorTreeWrapperPassPass(
      *PassRegistry::getPassRegistry());
}

char &toolchain::MachineDominatorsID = MachineDominatorTreeWrapperPass::ID;

bool MachineDominatorTreeWrapperPass::runOnMachineFunction(MachineFunction &F) {
  DT = MachineDominatorTree(F);
  return false;
}

void MachineDominatorTreeWrapperPass::releaseMemory() { DT.reset(); }

void MachineDominatorTreeWrapperPass::verifyAnalysis() const {
  if (VerifyMachineDomInfo && DT)
    if (!DT->verify(MachineDominatorTree::VerificationLevel::Basic))
      report_fatal_error("MachineDominatorTree verification failed!");
}

void MachineDominatorTreeWrapperPass::print(raw_ostream &OS,
                                            const Module *) const {
  if (DT)
    DT->print(OS);
}
