//===- MachinePostDominators.cpp -Machine Post Dominator Calculation ------===//
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
// post dominators on machine functions.
//
//===----------------------------------------------------------------------===//

#include "vm/core/CodeGen/MachinePostDominators.h"
#include "vm/core/InitializePasses.h"
#include "vm/core/Support/GenericDomTreeConstruction.h"

using namespace vm::core;

namespace vm::core {
template class LLVM_EXPORT_TEMPLATE
    DominatorTreeBase<MachineBasicBlock, true>; // PostDomTreeBase

namespace DomTreeBuilder {

template LLVM_EXPORT_TEMPLATE void
Calculate<MBBPostDomTree>(MBBPostDomTree &DT);
template LLVM_EXPORT_TEMPLATE void
InsertEdge<MBBPostDomTree>(MBBPostDomTree &DT, MachineBasicBlock *From,
                           MachineBasicBlock *To);
template LLVM_EXPORT_TEMPLATE void
DeleteEdge<MBBPostDomTree>(MBBPostDomTree &DT, MachineBasicBlock *From,
                           MachineBasicBlock *To);
template LLVM_EXPORT_TEMPLATE void
ApplyUpdates<MBBPostDomTree>(MBBPostDomTree &DT, MBBPostDomTreeGraphDiff &,
                             MBBPostDomTreeGraphDiff *);
template LLVM_EXPORT_TEMPLATE bool
Verify<MBBPostDomTree>(const MBBPostDomTree &DT,
                       MBBPostDomTree::VerificationLevel VL);

} // namespace DomTreeBuilder
extern bool VerifyMachineDomInfo;
} // namespace vm::core

AnalysisKey MachinePostDominatorTreeAnalysis::Key;

MachinePostDominatorTreeAnalysis::Result
MachinePostDominatorTreeAnalysis::run(MachineFunction &MF,
                                      MachineFunctionAnalysisManager &) {
  return MachinePostDominatorTree(MF);
}

PreservedAnalyses
MachinePostDominatorTreePrinterPass::run(MachineFunction &MF,
                                         MachineFunctionAnalysisManager &MFAM) {
  OS << "MachinePostDominatorTree for machine function: " << MF.getName()
     << '\n';
  MFAM.getResult<MachinePostDominatorTreeAnalysis>(MF).print(OS);
  return PreservedAnalyses::all();
}

char MachinePostDominatorTreeWrapperPass::ID = 0;

//declare initializeMachinePostDominatorTreePass
INITIALIZE_PASS(MachinePostDominatorTreeWrapperPass, "machinepostdomtree",
                "MachinePostDominator Tree Construction", true, true)

MachinePostDominatorTreeWrapperPass::MachinePostDominatorTreeWrapperPass()
    : MachineFunctionPass(ID), PDT() {
  initializeMachinePostDominatorTreeWrapperPassPass(
      *PassRegistry::getPassRegistry());
}

bool MachinePostDominatorTreeWrapperPass::runOnMachineFunction(
    MachineFunction &F) {
  PDT = MachinePostDominatorTree();
  PDT->recalculate(F);
  return false;
}

void MachinePostDominatorTreeWrapperPass::getAnalysisUsage(
    AnalysisUsage &AU) const {
  AU.setPreservesAll();
  MachineFunctionPass::getAnalysisUsage(AU);
}

bool MachinePostDominatorTree::invalidate(
    MachineFunction &, const PreservedAnalyses &PA,
    MachineFunctionAnalysisManager::Invalidator &) {
  // Check whether the analysis, all analyses on machine functions, or the
  // machine function's CFG have been preserved.
  auto PAC = PA.getChecker<MachinePostDominatorTreeAnalysis>();
  return !PAC.preserved() &&
         !PAC.preservedSet<AllAnalysesOn<MachineFunction>>() &&
         !PAC.preservedSet<CFGAnalyses>();
}

MachineBasicBlock *MachinePostDominatorTree::findNearestCommonDominator(
    ArrayRef<MachineBasicBlock *> Blocks) const {
  assert(!Blocks.empty());

  MachineBasicBlock *NCD = Blocks.consume_front();
  for (MachineBasicBlock *BB : Blocks) {
    NCD = Base::findNearestCommonDominator(NCD, BB);

    // Stop when the root is reached.
    if (isVirtualRoot(getNode(NCD)))
      return nullptr;
  }

  return NCD;
}

void MachinePostDominatorTreeWrapperPass::verifyAnalysis() const {
  if (VerifyMachineDomInfo && PDT &&
      !PDT->verify(MachinePostDominatorTree::VerificationLevel::Basic))
    report_fatal_error("MachinePostDominatorTree verification failed!");
}

void MachinePostDominatorTreeWrapperPass::print(toolchain::raw_ostream &OS,
                                                const Module *M) const {
  PDT->print(OS);
}
