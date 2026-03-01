//===-- Thumb1InstrInfo.cpp - Thumb-1 Instruction Information -------------===//
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
// This file contains the Thumb-1 implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//

#include "Thumb1InstrInfo.h"
#include "ARMSubtarget.h"
#include "vm/core/ADT/BitVector.h"
#include "vm/core/CodeGen/LiveRegUnits.h"
#include "vm/core/CodeGen/MachineFrameInfo.h"
#include "vm/core/CodeGen/MachineInstrBuilder.h"
#include "vm/core/CodeGen/MachineMemOperand.h"
#include "vm/core/IR/Module.h"
#include "vm/core/MC/MCInst.h"
#include "vm/core/MC/MCInstBuilder.h"

using namespace vm::core;

Thumb1InstrInfo::Thumb1InstrInfo(const ARMSubtarget &STI)
    : ARMBaseInstrInfo(STI, RI), RI(STI) {}

/// Return the noop instruction to use for a noop.
MCInst Thumb1InstrInfo::getNop() const {
  return MCInstBuilder(ARM::tMOVr)
      .addReg(ARM::R8)
      .addReg(ARM::R8)
      .addImm(ARMCC::AL)
      .addReg(0);
}

unsigned Thumb1InstrInfo::getUnindexedOpcode(unsigned Opc) const {
  return 0;
}

void Thumb1InstrInfo::copyPhysReg(MachineBasicBlock &MBB,
                                  MachineBasicBlock::iterator I,
                                  const DebugLoc &DL, Register DestReg,
                                  Register SrcReg, bool KillSrc,
                                  bool RenamableDest, bool RenamableSrc) const {
  // Need to check the arch.
  MachineFunction &MF = *MBB.getParent();
  const ARMSubtarget &st = MF.getSubtarget<ARMSubtarget>();

  assert(ARM::GPRRegClass.contains(DestReg, SrcReg) &&
         "Thumb1 can only copy GPR registers");

  if (st.hasV6Ops() || ARM::hGPRRegClass.contains(SrcReg) ||
      !ARM::tGPRRegClass.contains(DestReg))
    BuildMI(MBB, I, DL, get(ARM::tMOVr), DestReg)
        .addReg(SrcReg, getKillRegState(KillSrc))
        .add(predOps(ARMCC::AL));
  else {
    const TargetRegisterInfo *RegInfo = st.getRegisterInfo();
    LiveRegUnits UsedRegs(*RegInfo);
    UsedRegs.addLiveOuts(MBB);

    auto InstUpToI = MBB.end();
    while (InstUpToI != I)
      // The pre-decrement is on purpose here.
      // We want to have the liveness right before I.
      UsedRegs.stepBackward(*--InstUpToI);

    if (UsedRegs.available(ARM::CPSR)) {
      BuildMI(MBB, I, DL, get(ARM::tMOVSr), DestReg)
          .addReg(SrcReg, getKillRegState(KillSrc))
          ->addRegisterDead(ARM::CPSR, RegInfo);
      return;
    }

    // Use high register to move source to destination
    // if movs is not an option.
    BitVector Allocatable = RegInfo->getAllocatableSet(
        MF, RegInfo->getRegClass(ARM::hGPRRegClassID));

    Register TmpReg = ARM::NoRegister;
    // Prefer R12 as it is known to not be preserved anyway
    if (UsedRegs.available(ARM::R12) && Allocatable.test(ARM::R12)) {
      TmpReg = ARM::R12;
    } else {
      for (Register Reg : Allocatable.set_bits()) {
        if (UsedRegs.available(Reg)) {
          TmpReg = Reg;
          break;
        }
      }
    }

    if (TmpReg) {
      BuildMI(MBB, I, DL, get(ARM::tMOVr), TmpReg)
          .addReg(SrcReg, getKillRegState(KillSrc))
          .add(predOps(ARMCC::AL));
      BuildMI(MBB, I, DL, get(ARM::tMOVr), DestReg)
          .addReg(TmpReg, getKillRegState(true))
          .add(predOps(ARMCC::AL));
      return;
    }

    // 'MOV lo, lo' is unpredictable on < v6, so use the stack to do it
    BuildMI(MBB, I, DL, get(ARM::tPUSH))
        .add(predOps(ARMCC::AL))
        .addReg(SrcReg, getKillRegState(KillSrc));
    BuildMI(MBB, I, DL, get(ARM::tPOP))
        .add(predOps(ARMCC::AL))
        .addReg(DestReg, getDefRegState(true));
  }
}

void Thumb1InstrInfo::storeRegToStackSlot(MachineBasicBlock &MBB,
                                          MachineBasicBlock::iterator I,
                                          Register SrcReg, bool isKill, int FI,
                                          const TargetRegisterClass *RC,
                                          Register VReg,
                                          MachineInstr::MIFlag Flags) const {
  assert((RC == &ARM::tGPRRegClass ||
          (SrcReg.isPhysical() && isARMLowRegister(SrcReg))) &&
         "Unknown regclass!");

  if (RC == &ARM::tGPRRegClass ||
      (SrcReg.isPhysical() && isARMLowRegister(SrcReg))) {
    DebugLoc DL;
    if (I != MBB.end()) DL = I->getDebugLoc();

    MachineFunction &MF = *MBB.getParent();
    MachineFrameInfo &MFI = MF.getFrameInfo();
    MachineMemOperand *MMO = MF.getMachineMemOperand(
        MachinePointerInfo::getFixedStack(MF, FI), MachineMemOperand::MOStore,
        MFI.getObjectSize(FI), MFI.getObjectAlign(FI));
    BuildMI(MBB, I, DL, get(ARM::tSTRspi))
        .addReg(SrcReg, getKillRegState(isKill))
        .addFrameIndex(FI)
        .addImm(0)
        .addMemOperand(MMO)
        .add(predOps(ARMCC::AL));
  }
}

void Thumb1InstrInfo::loadRegFromStackSlot(MachineBasicBlock &MBB,
                                           MachineBasicBlock::iterator I,
                                           Register DestReg, int FI,
                                           const TargetRegisterClass *RC,
                                           Register VReg, unsigned SubReg,
                                           MachineInstr::MIFlag Flags) const {
  assert((RC->hasSuperClassEq(&ARM::tGPRRegClass) ||
          (DestReg.isPhysical() && isARMLowRegister(DestReg))) &&
         "Unknown regclass!");

  if (RC->hasSuperClassEq(&ARM::tGPRRegClass) ||
      (DestReg.isPhysical() && isARMLowRegister(DestReg))) {
    DebugLoc DL;
    if (I != MBB.end()) DL = I->getDebugLoc();

    MachineFunction &MF = *MBB.getParent();
    MachineFrameInfo &MFI = MF.getFrameInfo();
    MachineMemOperand *MMO = MF.getMachineMemOperand(
        MachinePointerInfo::getFixedStack(MF, FI), MachineMemOperand::MOLoad,
        MFI.getObjectSize(FI), MFI.getObjectAlign(FI));
    BuildMI(MBB, I, DL, get(ARM::tLDRspi), DestReg)
        .addFrameIndex(FI)
        .addImm(0)
        .addMemOperand(MMO)
        .add(predOps(ARMCC::AL));
  }
}

void Thumb1InstrInfo::expandLoadStackGuard(
    MachineBasicBlock::iterator MI) const {
  MachineFunction &MF = *MI->getParent()->getParent();
  const ARMSubtarget &ST = MF.getSubtarget<ARMSubtarget>();
  const auto *GV = cast<GlobalValue>((*MI->memoperands_begin())->getValue());

  assert(MF.getFunction().getParent()->getStackProtectorGuard() != "tls" &&
         "TLS stack protector not supported for Thumb1 targets");

  unsigned Instr;
  if (!GV->isDSOLocal())
    Instr = ARM::tLDRLIT_ga_pcrel;
  else if (ST.genExecuteOnly() && ST.hasV8MBaselineOps())
    Instr = ARM::t2MOVi32imm;
  else if (ST.genExecuteOnly())
    Instr = ARM::tMOVi32imm;
  else
    Instr = ARM::tLDRLIT_ga_abs;
  expandLoadStackGuardBase(MI, Instr, ARM::tLDRi);
}

bool Thumb1InstrInfo::canCopyGluedNodeDuringSchedule(SDNode *N) const {
  // In Thumb1 the scheduler may need to schedule a cross-copy between GPRS and CPSR
  // but this is not always possible there, so allow the Scheduler to clone tADCS and tSBCS
  // even if they have glue.
  // FIXME. Actually implement the cross-copy where it is possible (post v6)
  // because these copies entail more spilling.
  unsigned Opcode = N->getMachineOpcode();
  if (Opcode == ARM::tADCS || Opcode == ARM::tSBCS)
    return true;

  return false;
}
