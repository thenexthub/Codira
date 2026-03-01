//===- MachineBranchProbabilityInfo.cpp - Machine Branch Probability Info -===//
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
// This analysis uses probability info stored in Machine Basic Blocks.
//
//===----------------------------------------------------------------------===//

#include "vm/core/CodeGen/MachineBranchProbabilityInfo.h"
#include "vm/core/CodeGen/MachineBasicBlock.h"
#include "vm/core/InitializePasses.h"
#include "vm/core/Support/CommandLine.h"
#include "vm/core/Support/raw_ostream.h"

using namespace vm::core;

INITIALIZE_PASS(MachineBranchProbabilityInfoWrapperPass, "machine-branch-prob",
                "Machine Branch Probability Analysis", false, true)
namespace vm::core {
cl::opt<unsigned>
    StaticLikelyProb("static-likely-prob",
                     cl::desc("branch probability threshold in percentage"
                              " to be considered very likely"),
                     cl::init(80), cl::Hidden);

cl::opt<unsigned> ProfileLikelyProb(
    "profile-likely-prob",
    cl::desc("branch probability threshold in percentage to be considered"
             " very likely when profile is available"),
    cl::init(51), cl::Hidden);
} // namespace vm::core

MachineBranchProbabilityAnalysis::Result
MachineBranchProbabilityAnalysis::run(MachineFunction &,
                                      MachineFunctionAnalysisManager &) {
  return MachineBranchProbabilityInfo();
}

PreservedAnalyses
MachineBranchProbabilityPrinterPass::run(MachineFunction &MF,
                                         MachineFunctionAnalysisManager &MFAM) {
  OS << "Printing analysis 'Machine Branch Probability Analysis' for machine "
        "function '"
     << MF.getName() << "':\n";
  auto &MBPI = MFAM.getResult<MachineBranchProbabilityAnalysis>(MF);
  for (const MachineBasicBlock &MBB : MF) {
    for (const MachineBasicBlock *Succ : MBB.successors())
      MBPI.printEdgeProbability(OS << "  ", &MBB, Succ);
  }
  return PreservedAnalyses::all();
}

char MachineBranchProbabilityInfoWrapperPass::ID = 0;

MachineBranchProbabilityInfoWrapperPass::
    MachineBranchProbabilityInfoWrapperPass()
    : ImmutablePass(ID) {
  PassRegistry &Registry = *PassRegistry::getPassRegistry();
  initializeMachineBranchProbabilityInfoWrapperPassPass(Registry);
}

void MachineBranchProbabilityInfoWrapperPass::anchor() {}

AnalysisKey MachineBranchProbabilityAnalysis::Key;

bool MachineBranchProbabilityInfo::invalidate(
    MachineFunction &, const PreservedAnalyses &PA,
    MachineFunctionAnalysisManager::Invalidator &) {
  auto PAC = PA.getChecker<MachineBranchProbabilityAnalysis>();
  return !PAC.preservedWhenStateless();
}

BranchProbability MachineBranchProbabilityInfo::getEdgeProbability(
    const MachineBasicBlock *Src,
    MachineBasicBlock::const_succ_iterator Dst) const {
  return Src->getSuccProbability(Dst);
}

BranchProbability MachineBranchProbabilityInfo::getEdgeProbability(
    const MachineBasicBlock *Src, const MachineBasicBlock *Dst) const {
  // This is a linear search. Try to use the const_succ_iterator version when
  // possible.
  return getEdgeProbability(Src, find(Src->successors(), Dst));
}

bool MachineBranchProbabilityInfo::isEdgeHot(
    const MachineBasicBlock *Src, const MachineBasicBlock *Dst) const {
  BranchProbability HotProb(StaticLikelyProb, 100);
  return getEdgeProbability(Src, Dst) > HotProb;
}

raw_ostream &MachineBranchProbabilityInfo::printEdgeProbability(
    raw_ostream &OS, const MachineBasicBlock *Src,
    const MachineBasicBlock *Dst) const {

  const BranchProbability Prob = getEdgeProbability(Src, Dst);
  OS << "edge " << printMBBReference(*Src) << " -> " << printMBBReference(*Dst)
     << " probability is " << Prob
     << (isEdgeHot(Src, Dst) ? " [HOT edge]\n" : "\n");

  return OS;
}
