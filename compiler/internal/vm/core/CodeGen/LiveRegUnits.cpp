//===- LiveRegUnits.cpp - Register Unit Set -------------------------------===//
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
/// \file This file imlements the LiveRegUnits set.
//
//===----------------------------------------------------------------------===//

#include "vm/core/CodeGen/LiveRegUnits.h"
#include "vm/core/CodeGen/MachineBasicBlock.h"
#include "vm/core/CodeGen/MachineFrameInfo.h"
#include "vm/core/CodeGen/MachineFunction.h"
#include "vm/core/CodeGen/MachineOperand.h"
#include "vm/core/CodeGen/MachineRegisterInfo.h"

using namespace vm::core;

void LiveRegUnits::removeRegsNotPreserved(const uint32_t *RegMask) {
  for (MCRegUnit U : TRI->regunits()) {
    for (MCRegUnitRootIterator RootReg(U, TRI); RootReg.isValid(); ++RootReg) {
      if (MachineOperand::clobbersPhysReg(RegMask, *RootReg)) {
        Units.reset(static_cast<unsigned>(U));
        break;
      }
    }
  }
}

void LiveRegUnits::addRegsInMask(const uint32_t *RegMask) {
  for (MCRegUnit U : TRI->regunits()) {
    for (MCRegUnitRootIterator RootReg(U, TRI); RootReg.isValid(); ++RootReg) {
      if (MachineOperand::clobbersPhysReg(RegMask, *RootReg)) {
        Units.set(static_cast<unsigned>(U));
        break;
      }
    }
  }
}

void LiveRegUnits::stepBackward(const MachineInstr &MI) {
  // Remove defined registers and regmask kills from the set.
  for (const MachineOperand &MOP : MI.operands()) {
    if (MOP.isReg()) {
      if (MOP.isDef() && MOP.getReg().isPhysical())
        removeReg(MOP.getReg());
      continue;
    }

    if (MOP.isRegMask()) {
      removeRegsNotPreserved(MOP.getRegMask());
      continue;
    }
  }

  // Add uses to the set.
  for (const MachineOperand &MOP : MI.operands()) {
    if (!MOP.isReg() || !MOP.readsReg())
      continue;

    if (MOP.getReg().isPhysical())
      addReg(MOP.getReg());
  }
}

void LiveRegUnits::accumulate(const MachineInstr &MI) {
  // Add defs, uses and regmask clobbers to the set.
  for (const MachineOperand &MOP : MI.operands()) {
    if (MOP.isReg()) {
      if (!MOP.getReg().isPhysical())
        continue;
      if (MOP.isDef() || MOP.readsReg())
        addReg(MOP.getReg());
      continue;
    }

    if (MOP.isRegMask()) {
      addRegsInMask(MOP.getRegMask());
      continue;
    }
  }
}

/// Add live-in registers of basic block \p MBB to \p LiveUnits.
static void addBlockLiveIns(LiveRegUnits &LiveUnits,
                            const MachineBasicBlock &MBB) {
  for (const auto &LI : MBB.liveins())
    LiveUnits.addRegMasked(LI.PhysReg, LI.LaneMask);
}

/// Add live-out registers of basic block \p MBB to \p LiveUnits.
static void addBlockLiveOuts(LiveRegUnits &LiveUnits,
                             const MachineBasicBlock &MBB) {
  for (const auto &LO : MBB.liveouts())
    LiveUnits.addRegMasked(LO.PhysReg, LO.LaneMask);
}

/// Adds all callee saved registers to \p LiveUnits.
static void addCalleeSavedRegs(LiveRegUnits &LiveUnits,
                               const MachineFunction &MF) {
  const MachineRegisterInfo &MRI = MF.getRegInfo();
  const MachineFrameInfo &MFI = MF.getFrameInfo();
  for (const MCPhysReg *CSR = MRI.getCalleeSavedRegs(); CSR && *CSR; ++CSR) {
    const unsigned N = *CSR;

    const auto &CSI = MFI.getCalleeSavedInfo();
    auto Info =
        toolchain::find_if(CSI, [N](auto Info) { return Info.getReg() == N; });
    // If we have no info for this callee-saved register, assume it is liveout
    if (Info == CSI.end() || Info->isRestored())
      LiveUnits.addReg(N);
  }
}

void LiveRegUnits::addPristines(const MachineFunction &MF) {
  const MachineFrameInfo &MFI = MF.getFrameInfo();
  if (!MFI.isCalleeSavedInfoValid())
    return;
  /// This function will usually be called on an empty object, handle this
  /// as a special case.
  if (empty()) {
    /// Add all callee saved regs, then remove the ones that are saved and
    /// restored.
    addCalleeSavedRegs(*this, MF);
    /// Remove the ones that are not saved/restored; they are pristine.
    for (const CalleeSavedInfo &Info : MFI.getCalleeSavedInfo())
      removeReg(Info.getReg());
    return;
  }
  /// If a callee-saved register that is not pristine is already present
  /// in the set, we should make sure that it stays in it. Precompute the
  /// set of pristine registers in a separate object.
  /// Add all callee saved regs, then remove the ones that are saved+restored.
  LiveRegUnits Pristine(*TRI);
  addCalleeSavedRegs(Pristine, MF);
  /// Remove the ones that are not saved/restored; they are pristine.
  for (const CalleeSavedInfo &Info : MFI.getCalleeSavedInfo())
    Pristine.removeReg(Info.getReg());
  addUnits(Pristine.getBitVector());
}

void LiveRegUnits::addLiveOuts(const MachineBasicBlock &MBB) {
  const MachineFunction &MF = *MBB.getParent();
  addPristines(MF);
  addBlockLiveOuts(*this, MBB);

  // For the return block: Add all callee saved registers.
  if (MBB.isReturnBlock()) {
    const MachineFrameInfo &MFI = MF.getFrameInfo();
    if (MFI.isCalleeSavedInfoValid())
      addCalleeSavedRegs(*this, MF);
  }
}

void LiveRegUnits::addLiveIns(const MachineBasicBlock &MBB) {
  const MachineFunction &MF = *MBB.getParent();
  addPristines(MF);
  addBlockLiveIns(*this, MBB);
}
