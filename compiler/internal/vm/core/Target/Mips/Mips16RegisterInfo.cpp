//===-- Mips16RegisterInfo.cpp - MIPS16 Register Information --------------===//
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
// This file contains the MIPS16 implementation of the TargetRegisterInfo class.
//
//===----------------------------------------------------------------------===//

#include "Mips16RegisterInfo.h"
#include "Mips16InstrInfo.h"
#include "MipsInstrInfo.h"
#include "vm/core/CodeGen/MachineFrameInfo.h"
#include "vm/core/CodeGen/MachineFunction.h"
#include "vm/core/CodeGen/TargetFrameLowering.h"
#include "vm/core/CodeGen/TargetInstrInfo.h"
#include "vm/core/IR/Function.h"
#include "vm/core/IR/Type.h"
#include "vm/core/Support/Debug.h"
#include "vm/core/Support/raw_ostream.h"
#include "vm/core/Target/TargetMachine.h"
#include "vm/core/Target/TargetOptions.h"

using namespace vm::core;

#define DEBUG_TYPE "mips16-registerinfo"

Mips16RegisterInfo::Mips16RegisterInfo(const MipsSubtarget &STI)
    : MipsRegisterInfo(STI) {}

bool Mips16RegisterInfo::requiresRegisterScavenging
  (const MachineFunction &MF) const {
  return false;
}
bool Mips16RegisterInfo::requiresFrameIndexScavenging
  (const MachineFunction &MF) const {
  return false;
}

bool Mips16RegisterInfo::useFPForScavengingIndex
  (const MachineFunction &MF) const {
  return false;
}

bool Mips16RegisterInfo::saveScavengerRegister(
    MachineBasicBlock &MBB, MachineBasicBlock::iterator I,
    MachineBasicBlock::iterator &UseMI, const TargetRegisterClass *RC,
    Register Reg) const {
  DebugLoc DL;
  const TargetInstrInfo &TII = *MBB.getParent()->getSubtarget().getInstrInfo();
  TII.copyPhysReg(MBB, I, DL, Mips::T0, Reg, true);
  TII.copyPhysReg(MBB, UseMI, DL, Reg, Mips::T0, true);
  return true;
}

const TargetRegisterClass *
Mips16RegisterInfo::intRegClass(unsigned Size) const {
  assert(Size == 4);
  return &Mips::CPU16RegsRegClass;
}

void Mips16RegisterInfo::eliminateFI(MachineBasicBlock::iterator II,
                                     unsigned OpNo, int FrameIndex,
                                     uint64_t StackSize,
                                     int64_t SPOffset) const {
  MachineInstr &MI = *II;
  MachineFunction &MF = *MI.getParent()->getParent();
  MachineFrameInfo &MFI = MF.getFrameInfo();

  const std::vector<CalleeSavedInfo> &CSI = MFI.getCalleeSavedInfo();
  int MinCSFI = 0;
  int MaxCSFI = -1;

  if (CSI.size()) {
    MinCSFI = CSI[0].getFrameIdx();
    MaxCSFI = CSI[CSI.size() - 1].getFrameIdx();
  }

  // The following stack frame objects are always
  // referenced relative to $sp:
  //  1. Outgoing arguments.
  //  2. Pointer to dynamically allocated stack space.
  //  3. Locations for callee-saved registers.
  // Everything else is referenced relative to whatever register
  // getFrameRegister() returns.
  Register FrameReg;

  if (FrameIndex >= MinCSFI && FrameIndex <= MaxCSFI)
    FrameReg = Mips::SP;
  else {
    const TargetFrameLowering *TFI = MF.getSubtarget().getFrameLowering();
    if (TFI->hasFP(MF)) {
      FrameReg = Mips::S0;
    }
    else {
      if ((MI.getNumOperands()> OpNo+2) && MI.getOperand(OpNo+2).isReg())
        FrameReg = MI.getOperand(OpNo+2).getReg();
      else
        FrameReg = Mips::SP;
    }
  }
  // Calculate final offset.
  // - There is no need to change the offset if the frame object
  //   is one of the
  //   following: an outgoing argument, pointer to a dynamically allocated
  //   stack space or a $gp restore location,
  // - If the frame object is any of the following,
  //   its offset must be adjusted
  //   by adding the size of the stack:
  //   incoming argument, callee-saved register location or local variable.
  int64_t Offset;
  bool IsKill = false;
  Offset = SPOffset + (int64_t)StackSize;
  Offset += MI.getOperand(OpNo + 1).getImm();

  LLVM_DEBUG(errs() << "Offset     : " << Offset << "\n"
                    << "<--------->\n");

  if (!MI.isDebugValue() &&
      !Mips16InstrInfo::validImmediate(MI.getOpcode(), FrameReg, Offset)) {
    MachineBasicBlock &MBB = *MI.getParent();
    DebugLoc DL = II->getDebugLoc();
    unsigned NewImm;
    const Mips16InstrInfo &TII =
        *static_cast<const Mips16InstrInfo *>(MF.getSubtarget().getInstrInfo());
    FrameReg = TII.loadImmediate(FrameReg, Offset, MBB, II, DL, NewImm);
    Offset = SignExtend64<16>(NewImm);
    IsKill = true;
  }
  MI.getOperand(OpNo).ChangeToRegister(FrameReg, false, false, IsKill);
  MI.getOperand(OpNo + 1).ChangeToImmediate(Offset);


}
