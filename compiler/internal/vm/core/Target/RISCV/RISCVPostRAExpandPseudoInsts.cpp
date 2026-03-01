//===-- RISCVPostRAExpandPseudoInsts.cpp - Expand pseudo instrs ----===//
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
// This file contains a pass that expands the pseudo instruction pseudolisimm32
// into target instructions. This pass should be run during the post-regalloc
// passes, before post RA scheduling.
//
//===----------------------------------------------------------------------===//

#include "RISCV.h"
#include "RISCVInstrInfo.h"
#include "vm/core/CodeGen/MachineFunctionPass.h"
#include "vm/core/CodeGen/MachineInstrBuilder.h"

using namespace vm::core;

#define RISCV_POST_RA_EXPAND_PSEUDO_NAME                                       \
  "RISC-V post-regalloc pseudo instruction expansion pass"

namespace {

class RISCVPostRAExpandPseudo : public MachineFunctionPass {
public:
  const RISCVInstrInfo *TII;
  static char ID;

  RISCVPostRAExpandPseudo() : MachineFunctionPass(ID) {}

  bool runOnMachineFunction(MachineFunction &MF) override;

  StringRef getPassName() const override {
    return RISCV_POST_RA_EXPAND_PSEUDO_NAME;
  }

private:
  bool expandMBB(MachineBasicBlock &MBB);
  bool expandMI(MachineBasicBlock &MBB, MachineBasicBlock::iterator MBBI,
                MachineBasicBlock::iterator &NextMBBI);
  bool expandMovImm(MachineBasicBlock &MBB, MachineBasicBlock::iterator MBBI);
  bool expandMovAddr(MachineBasicBlock &MBB, MachineBasicBlock::iterator MBBI);
};

char RISCVPostRAExpandPseudo::ID = 0;

bool RISCVPostRAExpandPseudo::runOnMachineFunction(MachineFunction &MF) {
  TII = static_cast<const RISCVInstrInfo *>(MF.getSubtarget().getInstrInfo());
  bool Modified = false;
  for (auto &MBB : MF)
    Modified |= expandMBB(MBB);
  return Modified;
}

bool RISCVPostRAExpandPseudo::expandMBB(MachineBasicBlock &MBB) {
  bool Modified = false;

  MachineBasicBlock::iterator MBBI = MBB.begin(), E = MBB.end();
  while (MBBI != E) {
    MachineBasicBlock::iterator NMBBI = std::next(MBBI);
    Modified |= expandMI(MBB, MBBI, NMBBI);
    MBBI = NMBBI;
  }

  return Modified;
}

bool RISCVPostRAExpandPseudo::expandMI(MachineBasicBlock &MBB,
                                       MachineBasicBlock::iterator MBBI,
                                       MachineBasicBlock::iterator &NextMBBI) {
  switch (MBBI->getOpcode()) {
  case RISCV::PseudoMovImm:
    return expandMovImm(MBB, MBBI);
  case RISCV::PseudoMovAddr:
    return expandMovAddr(MBB, MBBI);
  default:
    return false;
  }
}

bool RISCVPostRAExpandPseudo::expandMovImm(MachineBasicBlock &MBB,
                                           MachineBasicBlock::iterator MBBI) {
  DebugLoc DL = MBBI->getDebugLoc();

  int64_t Val = MBBI->getOperand(1).getImm();

  Register DstReg = MBBI->getOperand(0).getReg();
  bool DstIsDead = MBBI->getOperand(0).isDead();
  bool Renamable = MBBI->getOperand(0).isRenamable();

  TII->movImm(MBB, MBBI, DL, DstReg, Val, MachineInstr::NoFlags, Renamable,
              DstIsDead);

  MBBI->eraseFromParent();
  return true;
}

bool RISCVPostRAExpandPseudo::expandMovAddr(MachineBasicBlock &MBB,
                                            MachineBasicBlock::iterator MBBI) {
  DebugLoc DL = MBBI->getDebugLoc();

  Register DstReg = MBBI->getOperand(0).getReg();
  bool DstIsDead = MBBI->getOperand(0).isDead();
  bool Renamable = MBBI->getOperand(0).isRenamable();

  BuildMI(MBB, MBBI, DL, TII->get(RISCV::LUI))
      .addReg(DstReg, RegState::Define | getRenamableRegState(Renamable))
      .add(MBBI->getOperand(1));
  BuildMI(MBB, MBBI, DL, TII->get(RISCV::ADDI))
      .addReg(DstReg, RegState::Define | getDeadRegState(DstIsDead) |
                          getRenamableRegState(Renamable))
      .addReg(DstReg, RegState::Kill | getRenamableRegState(Renamable))
      .add(MBBI->getOperand(2));
  MBBI->eraseFromParent();
  return true;
}

} // end of anonymous namespace

INITIALIZE_PASS(RISCVPostRAExpandPseudo, "riscv-post-ra-expand-pseudo",
                RISCV_POST_RA_EXPAND_PSEUDO_NAME, false, false)
namespace vm::core {

FunctionPass *createRISCVPostRAExpandPseudoPass() {
  return new RISCVPostRAExpandPseudo();
}

} // end of namespace vm::core
