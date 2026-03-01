//===-- MVETailPredUtils.h - Tail predication utility functions -*- C++-*-===//
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
// This file contains utility functions for low overhead and tail predicated
// loops, shared between the ARMLowOverheadLoops pass and anywhere else that
// needs them.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_ARM_MVETAILPREDUTILS_H
#define LLVM_LIB_TARGET_ARM_MVETAILPREDUTILS_H

#include "vm/core/CodeGen/MachineInstr.h"
#include "vm/core/CodeGen/MachineInstrBuilder.h"
#include "vm/core/CodeGen/MachineOperand.h"
#include "vm/core/CodeGen/TargetInstrInfo.h"

namespace vm::core {

static inline unsigned VCTPOpcodeToLSTP(unsigned Opcode, bool IsDoLoop) {
  switch (Opcode) {
  default:
    llvm_unreachable("unhandled vctp opcode");
    break;
  case ARM::MVE_VCTP8:
    return IsDoLoop ? ARM::MVE_DLSTP_8 : ARM::MVE_WLSTP_8;
  case ARM::MVE_VCTP16:
    return IsDoLoop ? ARM::MVE_DLSTP_16 : ARM::MVE_WLSTP_16;
  case ARM::MVE_VCTP32:
    return IsDoLoop ? ARM::MVE_DLSTP_32 : ARM::MVE_WLSTP_32;
  case ARM::MVE_VCTP64:
    return IsDoLoop ? ARM::MVE_DLSTP_64 : ARM::MVE_WLSTP_64;
  }
  return 0;
}

static inline unsigned getTailPredVectorWidth(unsigned Opcode) {
  switch (Opcode) {
  default:
    llvm_unreachable("unhandled vctp opcode");
  case ARM::MVE_VCTP8:
    return 16;
  case ARM::MVE_VCTP16:
    return 8;
  case ARM::MVE_VCTP32:
    return 4;
  case ARM::MVE_VCTP64:
    return 2;
  }
  return 0;
}

static inline bool isVCTP(const MachineInstr *MI) {
  switch (MI->getOpcode()) {
  default:
    break;
  case ARM::MVE_VCTP8:
  case ARM::MVE_VCTP16:
  case ARM::MVE_VCTP32:
  case ARM::MVE_VCTP64:
    return true;
  }
  return false;
}

static inline bool isDoLoopStart(const MachineInstr &MI) {
  return MI.getOpcode() == ARM::t2DoLoopStart ||
         MI.getOpcode() == ARM::t2DoLoopStartTP;
}

static inline bool isWhileLoopStart(const MachineInstr &MI) {
  return MI.getOpcode() == ARM::t2WhileLoopStart ||
         MI.getOpcode() == ARM::t2WhileLoopStartLR ||
         MI.getOpcode() == ARM::t2WhileLoopStartTP;
}

static inline bool isLoopStart(const MachineInstr &MI) {
  return isDoLoopStart(MI) || isWhileLoopStart(MI);
}

// Return the TargetBB stored in a t2WhileLoopStartLR/t2WhileLoopStartTP.
inline MachineBasicBlock *getWhileLoopStartTargetBB(const MachineInstr &MI) {
  assert(isWhileLoopStart(MI) && "Expected WhileLoopStart!");
  unsigned Op = MI.getOpcode() == ARM::t2WhileLoopStartTP ? 3 : 2;
  return MI.getOperand(Op).getMBB();
}

// WhileLoopStart holds the exit block, so produce a subs Op0, Op1, 0 and then a
// beq that branches to the exit branch.
// If UseCmp is true, this will create a t2CMP instead of a t2SUBri, meaning the
// value of LR into the loop will not be setup. This is used if the LR setup is
// done via another means (via a t2DoLoopStart, for example).
inline void RevertWhileLoopStartLR(MachineInstr *MI, const TargetInstrInfo *TII,
                                   unsigned BrOpc = ARM::t2Bcc,
                                   bool UseCmp = false) {
  MachineBasicBlock *MBB = MI->getParent();
  assert((MI->getOpcode() == ARM::t2WhileLoopStartLR ||
          MI->getOpcode() == ARM::t2WhileLoopStartTP) &&
         "Only expected a t2WhileLoopStartLR/TP in RevertWhileLoopStartLR!");

  // Subs/Cmp
  if (UseCmp) {
    MachineInstrBuilder MIB =
        BuildMI(*MBB, MI, MI->getDebugLoc(), TII->get(ARM::t2CMPri));
    MIB.add(MI->getOperand(1));
    MIB.addImm(0);
    MIB.addImm(ARMCC::AL);
    MIB.addReg(ARM::NoRegister);
  } else {
    MachineInstrBuilder MIB =
        BuildMI(*MBB, MI, MI->getDebugLoc(), TII->get(ARM::t2SUBri));
    MIB.add(MI->getOperand(0));
    MIB.add(MI->getOperand(1));
    MIB.addImm(0);
    MIB.addImm(ARMCC::AL);
    MIB.addReg(ARM::NoRegister);
    MIB.addReg(ARM::CPSR, RegState::Define);
  }

  // Branch
  MachineInstrBuilder MIB =
      BuildMI(*MBB, MI, MI->getDebugLoc(), TII->get(BrOpc));
  MIB.addMBB(getWhileLoopStartTargetBB(*MI)); // branch target
  MIB.addImm(ARMCC::EQ);                      // condition code
  MIB.addReg(ARM::CPSR);

  MI->eraseFromParent();
}

inline void RevertDoLoopStart(MachineInstr *MI, const TargetInstrInfo *TII) {
  MachineBasicBlock *MBB = MI->getParent();
  BuildMI(*MBB, MI, MI->getDebugLoc(), TII->get(ARM::tMOVr))
      .add(MI->getOperand(0))
      .add(MI->getOperand(1))
      .add(predOps(ARMCC::AL));

  MI->eraseFromParent();
}

inline void RevertLoopDec(MachineInstr *MI, const TargetInstrInfo *TII,
                          bool SetFlags = false) {
  MachineBasicBlock *MBB = MI->getParent();

  MachineInstrBuilder MIB =
      BuildMI(*MBB, MI, MI->getDebugLoc(), TII->get(ARM::t2SUBri));
  MIB.add(MI->getOperand(0));
  MIB.add(MI->getOperand(1));
  MIB.add(MI->getOperand(2));
  MIB.addImm(ARMCC::AL);
  MIB.addReg(0);

  if (SetFlags) {
    MIB.addReg(ARM::CPSR);
    MIB->getOperand(5).setIsDef(true);
  } else
    MIB.addReg(0);

  MI->eraseFromParent();
}

// Generate a subs, or sub and cmp, and a branch instead of an LE.
inline void RevertLoopEnd(MachineInstr *MI, const TargetInstrInfo *TII,
                          unsigned BrOpc = ARM::t2Bcc, bool SkipCmp = false) {
  MachineBasicBlock *MBB = MI->getParent();

  // Create cmp
  if (!SkipCmp) {
    MachineInstrBuilder MIB =
        BuildMI(*MBB, MI, MI->getDebugLoc(), TII->get(ARM::t2CMPri));
    MIB.add(MI->getOperand(0));
    MIB.addImm(0);
    MIB.addImm(ARMCC::AL);
    MIB.addReg(ARM::NoRegister);
  }

  // Create bne
  MachineInstrBuilder MIB =
      BuildMI(*MBB, MI, MI->getDebugLoc(), TII->get(BrOpc));
  MIB.add(MI->getOperand(1)); // branch target
  MIB.addImm(ARMCC::NE);      // condition code
  MIB.addReg(ARM::CPSR);
  MI->eraseFromParent();
}

} // end namespace vm::core

#endif // LLVM_LIB_TARGET_ARM_MVETAILPREDUTILS_H
