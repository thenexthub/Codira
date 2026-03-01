//===- MipsMulMulBugPass.cpp - Mips VR4300 mulmul bugfix pass -------------===//
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
// Early revisions of the VR4300 have a hardware bug where two consecutive
// multiplications can produce an incorrect result in the second multiply.
//
// This pass scans for mul instructions in each basic block and inserts
// a nop whenever the following conditions are met:
//
// - The current instruction is a single or double-precision floating-point
//   mul instruction.
// - The next instruction is either a mul instruction (any kind)
//   or a branch instruction.
//===----------------------------------------------------------------------===//

#include "Mips.h"
#include "MipsInstrInfo.h"
#include "MipsSubtarget.h"
#include "vm/core/CodeGen/MachineBasicBlock.h"
#include "vm/core/CodeGen/MachineFunction.h"
#include "vm/core/CodeGen/MachineFunctionPass.h"
#include "vm/core/Support/Debug.h"
#include "vm/core/Target/TargetMachine.h"

#define DEBUG_TYPE "mips-vr4300-mulmul-fix"

using namespace vm::core;

namespace {

class MipsMulMulBugFix : public MachineFunctionPass {
public:
  MipsMulMulBugFix() : MachineFunctionPass(ID) {}

  StringRef getPassName() const override { return "Mips VR4300 mulmul bugfix"; }

  MachineFunctionProperties getRequiredProperties() const override {
    return MachineFunctionProperties().setNoVRegs();
  }

  bool runOnMachineFunction(MachineFunction &MF) override;

  static char ID;

private:
  bool fixMulMulBB(MachineBasicBlock &MBB, const MipsInstrInfo &MipsII);
};

} // namespace

INITIALIZE_PASS(MipsMulMulBugFix, "mips-vr4300-mulmul-fix",
                "Mips VR4300 mulmul bugfix", false, false)

char MipsMulMulBugFix::ID = 0;

bool MipsMulMulBugFix::runOnMachineFunction(MachineFunction &MF) {
  const MipsInstrInfo &MipsII =
      *static_cast<const MipsInstrInfo *>(MF.getSubtarget().getInstrInfo());

  bool Modified = false;

  for (auto &MBB : MF)
    Modified |= fixMulMulBB(MBB, MipsII);

  return Modified;
}

static bool isFirstMul(const MachineInstr &MI) {
  switch (MI.getOpcode()) {
  case Mips::FMUL_S:
  case Mips::FMUL_D:
  case Mips::FMUL_D32:
  case Mips::FMUL_D64:
    return true;
  default:
    return false;
  }
}

static bool isSecondMulOrBranch(const MachineInstr &MI) {
  if (MI.isBranch() || MI.isIndirectBranch() || MI.isCall())
    return true;

  switch (MI.getOpcode()) {
  case Mips::MUL:
  case Mips::FMUL_S:
  case Mips::FMUL_D:
  case Mips::FMUL_D32:
  case Mips::FMUL_D64:
  case Mips::MULT:
  case Mips::MULTu:
  case Mips::DMULT:
  case Mips::DMULTu:
    return true;
  default:
    return false;
  }
}

bool MipsMulMulBugFix::fixMulMulBB(MachineBasicBlock &MBB,
                                   const MipsInstrInfo &MipsII) {
  bool Modified = false;

  MachineBasicBlock::instr_iterator NextMII;

  // Iterate through the instructions in the basic block
  for (MachineBasicBlock::instr_iterator MII = MBB.instr_begin(),
                                         E = MBB.instr_end();
       MII != E; MII = NextMII) {

    NextMII = next_nodbg(MII, E);

    // Trigger when the current instruction is a mul and the next instruction
    // is either a mul or a branch in case the branch target start with a mul
    if (NextMII != E && isFirstMul(*MII) && isSecondMulOrBranch(*NextMII)) {
      LLVM_DEBUG(dbgs() << "Found mulmul!\n");

      const MCInstrDesc &NewMCID = MipsII.get(Mips::NOP);
      BuildMI(MBB, NextMII, DebugLoc(), NewMCID);
      Modified = true;
    }
  }

  return Modified;
}

FunctionPass *toolchain::createMipsMulMulBugPass() { return new MipsMulMulBugFix(); }
