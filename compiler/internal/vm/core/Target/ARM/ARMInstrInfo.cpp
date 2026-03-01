//===-- ARMInstrInfo.cpp - ARM Instruction Information --------------------===//
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
// This file contains the ARM implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//

#include "ARMInstrInfo.h"
#include "ARM.h"
#include "ARMConstantPoolValue.h"
#include "ARMMachineFunctionInfo.h"
#include "ARMTargetMachine.h"
#include "vm/core/CodeGen/LiveVariables.h"
#include "vm/core/CodeGen/MachineFrameInfo.h"
#include "vm/core/CodeGen/MachineInstrBuilder.h"
#include "vm/core/CodeGen/MachineJumpTableInfo.h"
#include "vm/core/IR/Function.h"
#include "vm/core/IR/GlobalVariable.h"
#include "vm/core/IR/Module.h"
#include "vm/core/MC/MCInst.h"
using namespace vm::core;

ARMInstrInfo::ARMInstrInfo(const ARMSubtarget &STI)
    : ARMBaseInstrInfo(STI, RI) {}

/// Return the noop instruction to use for a noop.
MCInst ARMInstrInfo::getNop() const {
  MCInst NopInst;
  if (hasNOP()) {
    NopInst.setOpcode(ARM::HINT);
    NopInst.addOperand(MCOperand::createImm(0));
    NopInst.addOperand(MCOperand::createImm(ARMCC::AL));
    NopInst.addOperand(MCOperand::createReg(0));
  } else {
    NopInst.setOpcode(ARM::MOVr);
    NopInst.addOperand(MCOperand::createReg(ARM::R0));
    NopInst.addOperand(MCOperand::createReg(ARM::R0));
    NopInst.addOperand(MCOperand::createImm(ARMCC::AL));
    NopInst.addOperand(MCOperand::createReg(0));
    NopInst.addOperand(MCOperand::createReg(0));
  }
  return NopInst;
}

unsigned ARMInstrInfo::getUnindexedOpcode(unsigned Opc) const {
  switch (Opc) {
  default:
    break;
  case ARM::LDR_PRE_IMM:
  case ARM::LDR_PRE_REG:
  case ARM::LDR_POST_IMM:
  case ARM::LDR_POST_REG:
    return ARM::LDRi12;
  case ARM::LDRH_PRE:
  case ARM::LDRH_POST:
    return ARM::LDRH;
  case ARM::LDRB_PRE_IMM:
  case ARM::LDRB_PRE_REG:
  case ARM::LDRB_POST_IMM:
  case ARM::LDRB_POST_REG:
    return ARM::LDRBi12;
  case ARM::LDRSH_PRE:
  case ARM::LDRSH_POST:
    return ARM::LDRSH;
  case ARM::LDRSB_PRE:
  case ARM::LDRSB_POST:
    return ARM::LDRSB;
  case ARM::STR_PRE_IMM:
  case ARM::STR_PRE_REG:
  case ARM::STR_POST_IMM:
  case ARM::STR_POST_REG:
    return ARM::STRi12;
  case ARM::STRH_PRE:
  case ARM::STRH_POST:
    return ARM::STRH;
  case ARM::STRB_PRE_IMM:
  case ARM::STRB_PRE_REG:
  case ARM::STRB_POST_IMM:
  case ARM::STRB_POST_REG:
    return ARM::STRBi12;
  }

  return 0;
}

void ARMInstrInfo::expandLoadStackGuard(MachineBasicBlock::iterator MI) const {
  MachineFunction &MF = *MI->getParent()->getParent();
  const ARMSubtarget &Subtarget = MF.getSubtarget<ARMSubtarget>();
  const TargetMachine &TM = MF.getTarget();
  Module &M = *MF.getFunction().getParent();

  if (M.getStackProtectorGuard() == "tls") {
    expandLoadStackGuardBase(MI, ARM::MRC, ARM::LDRi12);
    return;
  }

  const GlobalValue *GV =
      cast<GlobalValue>((*MI->memoperands_begin())->getValue());

  bool ForceELFGOTPIC = Subtarget.isTargetELF() && !GV->isDSOLocal();
  if (!Subtarget.useMovt() || ForceELFGOTPIC) {
    // For ELF non-PIC, use GOT PIC code sequence as well because R_ARM_GOT_ABS
    // does not have assembler support.
    if (TM.isPositionIndependent() || ForceELFGOTPIC)
      expandLoadStackGuardBase(MI, ARM::LDRLIT_ga_pcrel, ARM::LDRi12);
    else
      expandLoadStackGuardBase(MI, ARM::LDRLIT_ga_abs, ARM::LDRi12);
    return;
  }

  if (!TM.isPositionIndependent()) {
    expandLoadStackGuardBase(MI, ARM::MOVi32imm, ARM::LDRi12);
    return;
  }

  if (!Subtarget.isGVIndirectSymbol(GV)) {
    expandLoadStackGuardBase(MI, ARM::MOV_ga_pcrel, ARM::LDRi12);
    return;
  }

  MachineBasicBlock &MBB = *MI->getParent();
  DebugLoc DL = MI->getDebugLoc();
  Register Reg = MI->getOperand(0).getReg();
  MachineInstrBuilder MIB;

  MIB = BuildMI(MBB, MI, DL, get(ARM::MOV_ga_pcrel_ldr), Reg)
            .addGlobalAddress(GV, 0, ARMII::MO_NONLAZY);
  auto Flags = MachineMemOperand::MOLoad |
               MachineMemOperand::MODereferenceable |
               MachineMemOperand::MOInvariant;
  MachineMemOperand *MMO = MBB.getParent()->getMachineMemOperand(
      MachinePointerInfo::getGOT(*MBB.getParent()), Flags, 4, Align(4));
  MIB.addMemOperand(MMO);
  BuildMI(MBB, MI, DL, get(ARM::LDRi12), Reg)
      .addReg(Reg, RegState::Kill)
      .addImm(0)
      .cloneMemRefs(*MI)
      .add(predOps(ARMCC::AL));
}
