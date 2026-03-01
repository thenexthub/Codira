//===- AMDGPUSetWavePriority.cpp - Set wave priority ----------------------===//
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
/// \file
/// Pass to temporarily raise the wave priority beginning the start of
/// the shader function until its last VMEM instructions to allow younger
/// waves to issue their VMEM instructions as well.
//
//===----------------------------------------------------------------------===//

#include "AMDGPU.h"
#include "GCNSubtarget.h"
#include "MCTargetDesc/AMDGPUMCTargetDesc.h"
#include "SIInstrInfo.h"
#include "vm/core/ADT/PostOrderIterator.h"
#include "vm/core/CodeGen/MachineFunctionPass.h"
#include "vm/core/CodeGen/MachinePassManager.h"

using namespace vm::core;

#define DEBUG_TYPE "amdgpu-set-wave-priority"

static cl::opt<unsigned> DefaultVALUInstsThreshold(
    "amdgpu-set-wave-priority-valu-insts-threshold",
    cl::desc("VALU instruction count threshold for adjusting wave priority"),
    cl::init(100), cl::Hidden);

namespace {

struct MBBInfo {
  MBBInfo() = default;
  unsigned NumVALUInstsAtStart = 0;
  bool MayReachVMEMLoad = false;
  MachineInstr *LastVMEMLoad = nullptr;
};

using MBBInfoSet = DenseMap<const MachineBasicBlock *, MBBInfo>;

class AMDGPUSetWavePriority {
public:
  bool run(MachineFunction &MF);

private:
  MachineInstr *BuildSetprioMI(MachineBasicBlock &MBB,
                               MachineBasicBlock::iterator I,
                               unsigned priority) const;

  const SIInstrInfo *TII;
};

class AMDGPUSetWavePriorityLegacy : public MachineFunctionPass {
public:
  static char ID;

  AMDGPUSetWavePriorityLegacy() : MachineFunctionPass(ID) {}

  StringRef getPassName() const override { return "Set wave priority"; }

  bool runOnMachineFunction(MachineFunction &MF) override {
    if (skipFunction(MF.getFunction()))
      return false;

    return AMDGPUSetWavePriority().run(MF);
  }
};

} // End anonymous namespace.

INITIALIZE_PASS(AMDGPUSetWavePriorityLegacy, DEBUG_TYPE, "Set wave priority",
                false, false)

char AMDGPUSetWavePriorityLegacy::ID = 0;

FunctionPass *toolchain::createAMDGPUSetWavePriorityPass() {
  return new AMDGPUSetWavePriorityLegacy();
}

MachineInstr *
AMDGPUSetWavePriority::BuildSetprioMI(MachineBasicBlock &MBB,
                                      MachineBasicBlock::iterator I,
                                      unsigned priority) const {
  return BuildMI(MBB, I, DebugLoc(), TII->get(AMDGPU::S_SETPRIO))
      .addImm(priority);
}

// Checks that for every predecessor Pred that can reach a VMEM load,
// none of Pred's successors can reach a VMEM load.
static bool CanLowerPriorityDirectlyInPredecessors(const MachineBasicBlock &MBB,
                                                   MBBInfoSet &MBBInfos) {
  for (const MachineBasicBlock *Pred : MBB.predecessors()) {
    if (!MBBInfos[Pred].MayReachVMEMLoad)
      continue;
    for (const MachineBasicBlock *Succ : Pred->successors()) {
      if (MBBInfos[Succ].MayReachVMEMLoad)
        return false;
    }
  }
  return true;
}

static bool isVMEMLoad(const MachineInstr &MI) {
  return SIInstrInfo::isVMEM(MI) && MI.mayLoad();
}

PreservedAnalyses
toolchain::AMDGPUSetWavePriorityPass::run(MachineFunction &MF,
                                     MachineFunctionAnalysisManager &MFAM) {
  if (!AMDGPUSetWavePriority().run(MF))
    return PreservedAnalyses::all();

  return getMachineFunctionPassPreservedAnalyses();
}

bool AMDGPUSetWavePriority::run(MachineFunction &MF) {
  const unsigned HighPriority = 3;
  const unsigned LowPriority = 0;

  Function &F = MF.getFunction();
  if (!AMDGPU::isEntryFunctionCC(F.getCallingConv()))
    return false;

  const GCNSubtarget &ST = MF.getSubtarget<GCNSubtarget>();
  TII = ST.getInstrInfo();

  unsigned VALUInstsThreshold = DefaultVALUInstsThreshold;
  Attribute A = F.getFnAttribute("amdgpu-wave-priority-threshold");
  if (A.isValid())
    A.getValueAsString().getAsInteger(0, VALUInstsThreshold);

  // Find VMEM loads that may be executed before long-enough sequences of
  // VALU instructions. We currently assume that backedges/loops, branch
  // probabilities and other details can be ignored, so we essentially
  // determine the largest number of VALU instructions along every
  // possible path from the start of the function that may potentially be
  // executed provided no backedge is ever taken.
  MBBInfoSet MBBInfos;
  for (MachineBasicBlock *MBB : post_order(&MF)) {
    bool AtStart = true;
    unsigned MaxNumVALUInstsInMiddle = 0;
    unsigned NumVALUInstsAtEnd = 0;
    for (MachineInstr &MI : *MBB) {
      if (isVMEMLoad(MI)) {
        AtStart = false;
        MBBInfo &Info = MBBInfos[MBB];
        Info.NumVALUInstsAtStart = 0;
        MaxNumVALUInstsInMiddle = 0;
        NumVALUInstsAtEnd = 0;
        Info.LastVMEMLoad = &MI;
      } else if (SIInstrInfo::isDS(MI)) {
        AtStart = false;
        MaxNumVALUInstsInMiddle =
            std::max(MaxNumVALUInstsInMiddle, NumVALUInstsAtEnd);
        NumVALUInstsAtEnd = 0;
      } else if (SIInstrInfo::isVALU(MI)) {
        if (AtStart)
          ++MBBInfos[MBB].NumVALUInstsAtStart;
        ++NumVALUInstsAtEnd;
      }
    }

    bool SuccsMayReachVMEMLoad = false;
    unsigned NumFollowingVALUInsts = 0;
    for (const MachineBasicBlock *Succ : MBB->successors()) {
      const MBBInfo &SuccInfo = MBBInfos[Succ];
      SuccsMayReachVMEMLoad |= SuccInfo.MayReachVMEMLoad;
      NumFollowingVALUInsts =
          std::max(NumFollowingVALUInsts, SuccInfo.NumVALUInstsAtStart);
    }
    MBBInfo &Info = MBBInfos[MBB];
    if (AtStart)
      Info.NumVALUInstsAtStart += NumFollowingVALUInsts;
    NumVALUInstsAtEnd += NumFollowingVALUInsts;

    unsigned MaxNumVALUInsts =
        std::max(MaxNumVALUInstsInMiddle, NumVALUInstsAtEnd);
    Info.MayReachVMEMLoad =
        SuccsMayReachVMEMLoad ||
        (Info.LastVMEMLoad && MaxNumVALUInsts >= VALUInstsThreshold);
  }

  MachineBasicBlock &Entry = MF.front();
  if (!MBBInfos[&Entry].MayReachVMEMLoad)
    return false;

  // Raise the priority at the beginning of the shader.
  MachineBasicBlock::iterator I = Entry.begin(), E = Entry.end();
  while (I != E && !SIInstrInfo::isVALU(*I) && !I->isTerminator())
    ++I;
  BuildSetprioMI(Entry, I, HighPriority);

  // Lower the priority on edges where control leaves blocks from which
  // the VMEM loads are reachable.
  SmallPtrSet<MachineBasicBlock *, 16> PriorityLoweringBlocks;
  for (MachineBasicBlock &MBB : MF) {
    if (MBBInfos[&MBB].MayReachVMEMLoad) {
      if (MBB.succ_empty())
        PriorityLoweringBlocks.insert(&MBB);
      continue;
    }

    if (CanLowerPriorityDirectlyInPredecessors(MBB, MBBInfos)) {
      for (MachineBasicBlock *Pred : MBB.predecessors()) {
        if (MBBInfos[Pred].MayReachVMEMLoad)
          PriorityLoweringBlocks.insert(Pred);
      }
      continue;
    }

    // Where lowering the priority in predecessors is not possible, the
    // block receiving control either was not part of a loop in the first
    // place or the loop simplification/canonicalization pass should have
    // already tried to split the edge and insert a preheader, and if for
    // whatever reason it failed to do so, then this leaves us with the
    // only option of lowering the priority within the loop.
    PriorityLoweringBlocks.insert(&MBB);
  }

  for (MachineBasicBlock *MBB : PriorityLoweringBlocks) {
    MachineInstr *LastVMEMLoad = MBBInfos[MBB].LastVMEMLoad;
    BuildSetprioMI(*MBB,
                   LastVMEMLoad
                       ? std::next(MachineBasicBlock::iterator(LastVMEMLoad))
                       : MBB->begin(),
                   LowPriority);
  }

  return true;
}
