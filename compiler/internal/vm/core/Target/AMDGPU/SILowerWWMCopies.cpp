//===-- SILowerWWMCopies.cpp - Lower Copies after regalloc ---===//
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
/// Lowering the WWM_COPY instructions for various register classes.
/// AMDGPU target generates WWM_COPY instruction to differentiate WWM
/// copy from COPY. This pass generates the necessary exec mask manipulation
/// instructions to replicate 'Whole Wave Mode' and lowers WWM_COPY back to
/// COPY.
//
//===----------------------------------------------------------------------===//

#include "SILowerWWMCopies.h"
#include "AMDGPU.h"
#include "GCNSubtarget.h"
#include "MCTargetDesc/AMDGPUMCTargetDesc.h"
#include "SIMachineFunctionInfo.h"
#include "vm/core/CodeGen/LiveIntervals.h"
#include "vm/core/CodeGen/MachineFunctionPass.h"
#include "vm/core/CodeGen/VirtRegMap.h"
#include "vm/core/InitializePasses.h"

using namespace vm::core;

#define DEBUG_TYPE "si-lower-wwm-copies"

namespace {

class SILowerWWMCopies {
public:
  SILowerWWMCopies(LiveIntervals *LIS, SlotIndexes *SI, VirtRegMap *VRM)
      : LIS(LIS), Indexes(SI), VRM(VRM) {}
  bool run(MachineFunction &MF);

private:
  bool isSCCLiveAtMI(const MachineInstr &MI);
  void addToWWMSpills(MachineFunction &MF, Register Reg);

  LiveIntervals *LIS;
  SlotIndexes *Indexes;
  VirtRegMap *VRM;
  const SIRegisterInfo *TRI;
  const MachineRegisterInfo *MRI;
  SIMachineFunctionInfo *MFI;
};

class SILowerWWMCopiesLegacy : public MachineFunctionPass {
public:
  static char ID;

  SILowerWWMCopiesLegacy() : MachineFunctionPass(ID) {
    initializeSILowerWWMCopiesLegacyPass(*PassRegistry::getPassRegistry());
  }

  bool runOnMachineFunction(MachineFunction &MF) override;

  StringRef getPassName() const override { return "SI Lower WWM Copies"; }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addUsedIfAvailable<LiveIntervalsWrapperPass>();
    AU.addUsedIfAvailable<SlotIndexesWrapperPass>();
    AU.addUsedIfAvailable<VirtRegMapWrapperLegacy>();
    AU.setPreservesAll();
    MachineFunctionPass::getAnalysisUsage(AU);
  }
};

} // End anonymous namespace.

INITIALIZE_PASS_BEGIN(SILowerWWMCopiesLegacy, DEBUG_TYPE, "SI Lower WWM Copies",
                      false, false)
INITIALIZE_PASS_DEPENDENCY(LiveIntervalsWrapperPass)
INITIALIZE_PASS_DEPENDENCY(VirtRegMapWrapperLegacy)
INITIALIZE_PASS_END(SILowerWWMCopiesLegacy, DEBUG_TYPE, "SI Lower WWM Copies",
                    false, false)

char SILowerWWMCopiesLegacy::ID = 0;

char &toolchain::SILowerWWMCopiesLegacyID = SILowerWWMCopiesLegacy::ID;

bool SILowerWWMCopies::isSCCLiveAtMI(const MachineInstr &MI) {
  // We can't determine the liveness info if LIS isn't available. Early return
  // in that case and always assume SCC is live.
  if (!LIS)
    return true;

  LiveRange &LR =
      LIS->getRegUnit(*MCRegUnitIterator(MCRegister::from(AMDGPU::SCC), TRI));
  SlotIndex Idx = LIS->getInstructionIndex(MI);
  return LR.liveAt(Idx);
}

// If \p Reg is assigned with a physical VGPR, add the latter into wwm-spills
// for preserving its entire lanes at function prolog/epilog.
void SILowerWWMCopies::addToWWMSpills(MachineFunction &MF, Register Reg) {
  if (Reg.isPhysical())
    return;

  // FIXME: VRM may be null here.
  MCRegister PhysReg = VRM->getPhys(Reg);
  assert(PhysReg && "should have allocated a physical register");

  MFI->allocateWWMSpill(MF, PhysReg);
}

bool SILowerWWMCopiesLegacy::runOnMachineFunction(MachineFunction &MF) {
  auto *LISWrapper = getAnalysisIfAvailable<LiveIntervalsWrapperPass>();
  auto *LIS = LISWrapper ? &LISWrapper->getLIS() : nullptr;

  auto *SIWrapper = getAnalysisIfAvailable<SlotIndexesWrapperPass>();
  auto *Indexes = SIWrapper ? &SIWrapper->getSI() : nullptr;

  auto *VRMWrapper = getAnalysisIfAvailable<VirtRegMapWrapperLegacy>();
  auto *VRM = VRMWrapper ? &VRMWrapper->getVRM() : nullptr;

  SILowerWWMCopies Impl(LIS, Indexes, VRM);
  return Impl.run(MF);
}

PreservedAnalyses
SILowerWWMCopiesPass::run(MachineFunction &MF,
                          MachineFunctionAnalysisManager &MFAM) {
  auto *LIS = MFAM.getCachedResult<LiveIntervalsAnalysis>(MF);
  auto *Indexes = MFAM.getCachedResult<SlotIndexesAnalysis>(MF);
  auto *VRM = MFAM.getCachedResult<VirtRegMapAnalysis>(MF);

  SILowerWWMCopies Impl(LIS, Indexes, VRM);
  Impl.run(MF);
  return PreservedAnalyses::all();
}

bool SILowerWWMCopies::run(MachineFunction &MF) {
  const GCNSubtarget &ST = MF.getSubtarget<GCNSubtarget>();
  const SIInstrInfo *TII = ST.getInstrInfo();

  MFI = MF.getInfo<SIMachineFunctionInfo>();
  TRI = ST.getRegisterInfo();
  MRI = &MF.getRegInfo();

  if (!MFI->hasVRegFlags())
    return false;

  bool Changed = false;
  for (MachineBasicBlock &MBB : MF) {
    for (MachineInstr &MI : MBB) {
      if (MI.getOpcode() != AMDGPU::WWM_COPY)
        continue;

      // TODO: Club adjacent WWM ops between same exec save/restore
      assert(TII->isVGPRCopy(MI));

      // For WWM vector copies, manipulate the exec mask around the copy
      // instruction.
      const DebugLoc &DL = MI.getDebugLoc();
      MachineBasicBlock::iterator InsertPt = MI.getIterator();
      Register RegForExecCopy = MFI->getSGPRForEXECCopy();
      TII->insertScratchExecCopy(MF, MBB, InsertPt, DL, RegForExecCopy,
                                 isSCCLiveAtMI(MI), Indexes);
      TII->restoreExec(MF, MBB, ++InsertPt, DL, RegForExecCopy, Indexes);
      addToWWMSpills(MF, MI.getOperand(0).getReg());
      LLVM_DEBUG(dbgs() << "WWM copy manipulation for " << MI);

      // Lower WWM_COPY back to COPY
      MI.setDesc(TII->get(AMDGPU::COPY));
      Changed |= true;
    }
  }

  return Changed;
}
