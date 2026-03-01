//===-- MSP430RegisterInfo.cpp - MSP430 Register Information --------------===//
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
// This file contains the MSP430 implementation of the TargetRegisterInfo class.
//
//===----------------------------------------------------------------------===//

#include "MSP430RegisterInfo.h"
#include "MSP430TargetMachine.h"
#include "vm/core/ADT/BitVector.h"
#include "vm/core/CodeGen/MachineFrameInfo.h"
#include "vm/core/CodeGen/MachineFunction.h"
#include "vm/core/CodeGen/MachineInstrBuilder.h"
#include "vm/core/IR/Function.h"
#include "vm/core/Target/TargetMachine.h"
#include "vm/core/Target/TargetOptions.h"

using namespace vm::core;

#define DEBUG_TYPE "msp430-reg-info"

#define GET_REGINFO_TARGET_DESC
#include "MSP430GenRegisterInfo.inc"

// FIXME: Provide proper call frame setup / destroy opcodes.
MSP430RegisterInfo::MSP430RegisterInfo()
  : MSP430GenRegisterInfo(MSP430::PC) {}

const MCPhysReg*
MSP430RegisterInfo::getCalleeSavedRegs(const MachineFunction *MF) const {
  const MSP430FrameLowering *TFI = getFrameLowering(*MF);
  const Function* F = &MF->getFunction();
  static const MCPhysReg CalleeSavedRegs[] = {
    MSP430::R4, MSP430::R5, MSP430::R6, MSP430::R7,
    MSP430::R8, MSP430::R9, MSP430::R10,
    0
  };
  static const MCPhysReg CalleeSavedRegsFP[] = {
    MSP430::R5, MSP430::R6, MSP430::R7,
    MSP430::R8, MSP430::R9, MSP430::R10,
    0
  };
  static const MCPhysReg CalleeSavedRegsIntr[] = {
    MSP430::R4,  MSP430::R5,  MSP430::R6,  MSP430::R7,
    MSP430::R8,  MSP430::R9,  MSP430::R10, MSP430::R11,
    MSP430::R12, MSP430::R13, MSP430::R14, MSP430::R15,
    0
  };
  static const MCPhysReg CalleeSavedRegsIntrFP[] = {
    MSP430::R5,  MSP430::R6,  MSP430::R7,
    MSP430::R8,  MSP430::R9,  MSP430::R10, MSP430::R11,
    MSP430::R12, MSP430::R13, MSP430::R14, MSP430::R15,
    0
  };

  if (TFI->hasFP(*MF))
    return (F->getCallingConv() == CallingConv::MSP430_INTR ?
            CalleeSavedRegsIntrFP : CalleeSavedRegsFP);
  else
    return (F->getCallingConv() == CallingConv::MSP430_INTR ?
            CalleeSavedRegsIntr : CalleeSavedRegs);

}

BitVector MSP430RegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  BitVector Reserved(getNumRegs());
  const MSP430FrameLowering *TFI = getFrameLowering(MF);

  // Mark 4 special registers with subregisters as reserved.
  Reserved.set(MSP430::PCB);
  Reserved.set(MSP430::SPB);
  Reserved.set(MSP430::SRB);
  Reserved.set(MSP430::CGB);
  Reserved.set(MSP430::PC);
  Reserved.set(MSP430::SP);
  Reserved.set(MSP430::SR);
  Reserved.set(MSP430::CG);

  // Mark frame pointer as reserved if needed.
  if (TFI->hasFP(MF)) {
    Reserved.set(MSP430::R4B);
    Reserved.set(MSP430::R4);
  }

  return Reserved;
}

const TargetRegisterClass *
MSP430RegisterInfo::getPointerRegClass(unsigned Kind) const {
  return &MSP430::GR16RegClass;
}

bool
MSP430RegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator II,
                                        int SPAdj, unsigned FIOperandNum,
                                        RegScavenger *RS) const {
  assert(SPAdj == 0 && "Unexpected");

  MachineInstr &MI = *II;
  MachineBasicBlock &MBB = *MI.getParent();
  MachineFunction &MF = *MBB.getParent();
  const MSP430FrameLowering *TFI = getFrameLowering(MF);
  DebugLoc dl = MI.getDebugLoc();
  int FrameIndex = MI.getOperand(FIOperandNum).getIndex();

  unsigned BasePtr = (TFI->hasFP(MF) ? MSP430::R4 : MSP430::SP);
  int Offset = MF.getFrameInfo().getObjectOffset(FrameIndex);

  // Skip the saved PC
  Offset += 2;

  if (!TFI->hasFP(MF))
    Offset += MF.getFrameInfo().getStackSize();
  else
    Offset += 2; // Skip the saved FP

  // Fold imm into offset
  Offset += MI.getOperand(FIOperandNum + 1).getImm();

  if (MI.getOpcode() == MSP430::ADDframe) {
    // This is actually "load effective address" of the stack slot
    // instruction. We have only two-address instructions, thus we need to
    // expand it into mov + add
    const TargetInstrInfo &TII = *MF.getSubtarget().getInstrInfo();

    MI.setDesc(TII.get(MSP430::MOV16rr));
    MI.getOperand(FIOperandNum).ChangeToRegister(BasePtr, false);

    // Remove the now unused Offset operand.
    MI.removeOperand(FIOperandNum + 1);

    if (Offset == 0)
      return false;

    // We need to materialize the offset via add instruction.
    Register DstReg = MI.getOperand(0).getReg();
    if (Offset < 0)
      BuildMI(MBB, std::next(II), dl, TII.get(MSP430::SUB16ri), DstReg)
        .addReg(DstReg).addImm(-Offset);
    else
      BuildMI(MBB, std::next(II), dl, TII.get(MSP430::ADD16ri), DstReg)
        .addReg(DstReg).addImm(Offset);

    return false;
  }

  MI.getOperand(FIOperandNum).ChangeToRegister(BasePtr, false);
  MI.getOperand(FIOperandNum + 1).ChangeToImmediate(Offset);
  return false;
}

Register MSP430RegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  const MSP430FrameLowering *TFI = getFrameLowering(MF);
  return TFI->hasFP(MF) ? MSP430::R4 : MSP430::SP;
}
