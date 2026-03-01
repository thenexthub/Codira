//===- Mips16FrameLowering.cpp - Mips16 Frame Information -----------------===//
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
// This file contains the Mips16 implementation of TargetFrameLowering class.
//
//===----------------------------------------------------------------------===//

#include "Mips16FrameLowering.h"
#include "Mips16InstrInfo.h"
#include "MipsInstrInfo.h"
#include "MipsRegisterInfo.h"
#include "MipsSubtarget.h"
#include "vm/core/ADT/BitVector.h"
#include "vm/core/CodeGen/CFIInstBuilder.h"
#include "vm/core/CodeGen/MachineBasicBlock.h"
#include "vm/core/CodeGen/MachineFrameInfo.h"
#include "vm/core/CodeGen/MachineFunction.h"
#include "vm/core/CodeGen/MachineInstr.h"
#include "vm/core/CodeGen/MachineInstrBuilder.h"
#include "vm/core/CodeGen/MachineModuleInfo.h"
#include "vm/core/CodeGen/TargetFrameLowering.h"
#include "vm/core/IR/DebugLoc.h"
#include "vm/core/Support/MathExtras.h"
#include <cstdint>
#include <vector>

using namespace vm::core;

Mips16FrameLowering::Mips16FrameLowering(const MipsSubtarget &STI)
    : MipsFrameLowering(STI, STI.getStackAlignment()) {}

void Mips16FrameLowering::emitPrologue(MachineFunction &MF,
                                       MachineBasicBlock &MBB) const {
  MachineFrameInfo &MFI = MF.getFrameInfo();
  const Mips16InstrInfo &TII =
      *static_cast<const Mips16InstrInfo *>(STI.getInstrInfo());
  MachineBasicBlock::iterator MBBI = MBB.begin();

  // Debug location must be unknown since the first debug location is used
  // to determine the end of the prologue.
  DebugLoc dl;

  uint64_t StackSize = MFI.getStackSize();

  // No need to allocate space on the stack.
  if (StackSize == 0 && !MFI.adjustsStack()) return;

  // Adjust stack.
  TII.makeFrame(Mips::SP, StackSize, MBB, MBBI);

  CFIInstBuilder CFIBuilder(MBB, MBBI, MachineInstr::NoFlags);
  CFIBuilder.buildDefCFAOffset(StackSize);

  const std::vector<CalleeSavedInfo> &CSI = MFI.getCalleeSavedInfo();

  if (!CSI.empty()) {
    for (const CalleeSavedInfo &I : CSI)
      CFIBuilder.buildOffset(I.getReg(), MFI.getObjectOffset(I.getFrameIdx()));
  }

  if (hasFP(MF))
    BuildMI(MBB, MBBI, dl, TII.get(Mips::MoveR3216), Mips::S0)
      .addReg(Mips::SP).setMIFlag(MachineInstr::FrameSetup);
}

void Mips16FrameLowering::emitEpilogue(MachineFunction &MF,
                                 MachineBasicBlock &MBB) const {
  MachineBasicBlock::iterator MBBI = MBB.getFirstTerminator();
  MachineFrameInfo &MFI = MF.getFrameInfo();
  const Mips16InstrInfo &TII =
      *static_cast<const Mips16InstrInfo *>(STI.getInstrInfo());
  DebugLoc dl = MBBI != MBB.end() ? MBBI->getDebugLoc() : DebugLoc();
  uint64_t StackSize = MFI.getStackSize();

  if (!StackSize)
    return;

  if (hasFP(MF))
    BuildMI(MBB, MBBI, dl, TII.get(Mips::Move32R16), Mips::SP)
      .addReg(Mips::S0);

  // Adjust stack.
  // assumes stacksize multiple of 8
  TII.restoreFrame(Mips::SP, StackSize, MBB, MBBI);
}

bool Mips16FrameLowering::spillCalleeSavedRegisters(
    MachineBasicBlock &MBB, MachineBasicBlock::iterator MI,
    ArrayRef<CalleeSavedInfo> CSI, const TargetRegisterInfo *TRI) const {
  MachineFunction *MF = MBB.getParent();

  //
  // Registers RA, S0,S1 are the callee saved registers and they
  // will be saved with the "save" instruction
  // during emitPrologue
  //
  for (const CalleeSavedInfo &I : CSI) {
    // Add the callee-saved register as live-in. Do not add if the register is
    // RA and return address is taken, because it has already been added in
    // method MipsTargetLowering::lowerRETURNADDR.
    // It's killed at the spill, unless the register is RA and return address
    // is taken.
    MCRegister Reg = I.getReg();
    bool IsRAAndRetAddrIsTaken = (Reg == Mips::RA)
      && MF->getFrameInfo().isReturnAddressTaken();
    if (!IsRAAndRetAddrIsTaken)
      MBB.addLiveIn(Reg);
  }

  return true;
}

bool Mips16FrameLowering::restoreCalleeSavedRegisters(
    MachineBasicBlock &MBB, MachineBasicBlock::iterator MI,
    MutableArrayRef<CalleeSavedInfo> CSI, const TargetRegisterInfo *TRI) const {
  //
  // Registers RA,S0,S1 are the callee saved registers and they will be restored
  // with the restore instruction during emitEpilogue.
  // We need to override this virtual function, otherwise toolchain will try and
  // restore the registers on it's on from the stack.
  //

  return true;
}

bool
Mips16FrameLowering::hasReservedCallFrame(const MachineFunction &MF) const {
  const MachineFrameInfo &MFI = MF.getFrameInfo();
  // Reserve call frame if the size of the maximum call frame fits into 15-bit
  // immediate field and there are no variable sized objects on the stack.
  return isInt<15>(MFI.getMaxCallFrameSize()) && !MFI.hasVarSizedObjects();
}

void Mips16FrameLowering::determineCalleeSaves(MachineFunction &MF,
                                               BitVector &SavedRegs,
                                               RegScavenger *RS) const {
  TargetFrameLowering::determineCalleeSaves(MF, SavedRegs, RS);
  const Mips16InstrInfo &TII =
      *static_cast<const Mips16InstrInfo *>(STI.getInstrInfo());
  const MipsRegisterInfo &RI = TII.getRegisterInfo();
  const BitVector Reserved = RI.getReservedRegs(MF);
  bool SaveS2 = Reserved[Mips::S2];
  if (SaveS2)
    SavedRegs.set(Mips::S2);
  if (hasFP(MF))
    SavedRegs.set(Mips::S0);
}

const MipsFrameLowering *
toolchain::createMips16FrameLowering(const MipsSubtarget &ST) {
  return new Mips16FrameLowering(ST);
}
