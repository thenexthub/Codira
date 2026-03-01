//===- lib/Codegen/MachineRegionInfo.cpp ----------------------------------===//
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

#include "vm/core/CodeGen/MachineRegionInfo.h"
#include "vm/core/ADT/Statistic.h"
#include "vm/core/Analysis/RegionInfoImpl.h"
#include "vm/core/CodeGen/MachinePostDominators.h"
#include "vm/core/CodeGen/Passes.h"
#include "vm/core/Config/toolchain-config.h"
#include "vm/core/InitializePasses.h"
#include "vm/core/Pass.h"
#include "vm/core/Support/Compiler.h"
#include "vm/core/Support/Debug.h"

#define DEBUG_TYPE "machine-region-info"

using namespace vm::core;

STATISTIC(numMachineRegions,       "The # of machine regions");
STATISTIC(numMachineSimpleRegions, "The # of simple machine regions");

namespace vm::core {

template class RegionBase<RegionTraits<MachineFunction>>;
template class RegionNodeBase<RegionTraits<MachineFunction>>;
template class RegionInfoBase<RegionTraits<MachineFunction>>;

} // end namespace vm::core

//===----------------------------------------------------------------------===//
// MachineRegion implementation

MachineRegion::MachineRegion(MachineBasicBlock *Entry, MachineBasicBlock *Exit,
                             MachineRegionInfo* RI,
                             MachineDominatorTree *DT, MachineRegion *Parent) :
  RegionBase<RegionTraits<MachineFunction>>(Entry, Exit, RI, DT, Parent) {}

MachineRegion::~MachineRegion() = default;

//===----------------------------------------------------------------------===//
// MachineRegionInfo implementation

MachineRegionInfo::MachineRegionInfo() = default;

MachineRegionInfo::~MachineRegionInfo() = default;

void MachineRegionInfo::updateStatistics(MachineRegion *R) {
  ++numMachineRegions;

  // TODO: Slow. Should only be enabled if -stats is used.
  if (R->isSimple())
    ++numMachineSimpleRegions;
}

void MachineRegionInfo::recalculate(MachineFunction &F,
                                    MachineDominatorTree *DT_,
                                    MachinePostDominatorTree *PDT_,
                                    MachineDominanceFrontier *DF_) {
  DT = DT_;
  PDT = PDT_;
  DF = DF_;

  MachineBasicBlock *Entry = GraphTraits<MachineFunction*>::getEntryNode(&F);

  TopLevelRegion = new MachineRegion(Entry, nullptr, this, DT, nullptr);
  updateStatistics(TopLevelRegion);
  calculate(F);
}

//===----------------------------------------------------------------------===//
// MachineRegionInfoPass implementation
//

MachineRegionInfoPass::MachineRegionInfoPass() : MachineFunctionPass(ID) {
  initializeMachineRegionInfoPassPass(*PassRegistry::getPassRegistry());
}

MachineRegionInfoPass::~MachineRegionInfoPass() = default;

bool MachineRegionInfoPass::runOnMachineFunction(MachineFunction &F) {
  releaseMemory();

  auto DT = &getAnalysis<MachineDominatorTreeWrapperPass>().getDomTree();
  auto PDT =
      &getAnalysis<MachinePostDominatorTreeWrapperPass>().getPostDomTree();
  auto DF = &getAnalysis<MachineDominanceFrontier>();

  RI.recalculate(F, DT, PDT, DF);

  LLVM_DEBUG(RI.dump());

  return false;
}

void MachineRegionInfoPass::releaseMemory() {
  RI.releaseMemory();
}

void MachineRegionInfoPass::verifyAnalysis() const {
  // Only do verification when user wants to, otherwise this expensive check
  // will be invoked by PMDataManager::verifyPreservedAnalysis when
  // a regionpass (marked PreservedAll) finish.
  if (MachineRegionInfo::VerifyRegionInfo)
    RI.verifyAnalysis();
}

void MachineRegionInfoPass::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequired<MachineDominatorTreeWrapperPass>();
  AU.addRequired<MachinePostDominatorTreeWrapperPass>();
  AU.addRequired<MachineDominanceFrontier>();
  MachineFunctionPass::getAnalysisUsage(AU);
}

void MachineRegionInfoPass::print(raw_ostream &OS, const Module *) const {
  RI.print(OS);
}

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD void MachineRegionInfoPass::dump() const {
  RI.dump();
}
#endif

char MachineRegionInfoPass::ID = 0;
char &toolchain::MachineRegionInfoPassID = MachineRegionInfoPass::ID;

INITIALIZE_PASS_BEGIN(MachineRegionInfoPass, DEBUG_TYPE,
                      "Detect single entry single exit regions", true, true)
INITIALIZE_PASS_DEPENDENCY(MachineDominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(MachinePostDominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(MachineDominanceFrontier)
INITIALIZE_PASS_END(MachineRegionInfoPass, DEBUG_TYPE,
                    "Detect single entry single exit regions", true, true)

// Create methods available outside of this file, to use them
// "include/toolchain/LinkAllPasses.h". Otherwise the pass would be deleted by
// the link time optimization.

namespace vm::core {

FunctionPass *createMachineRegionInfoPass() {
  return new MachineRegionInfoPass();
}

} // end namespace vm::core
