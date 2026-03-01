//===-- LVLGen.cpp - LVL instruction generator ----------------------------===//
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

#include "VE.h"
#include "VESubtarget.h"
#include "vm/core/CodeGen/MachineFunctionPass.h"
#include "vm/core/CodeGen/MachineInstrBuilder.h"
#include "vm/core/CodeGen/TargetInstrInfo.h"

using namespace vm::core;

#define DEBUG_TYPE "lvl-gen"

namespace {
struct LVLGen : public MachineFunctionPass {
  const TargetInstrInfo *TII;
  const TargetRegisterInfo *TRI;

  static char ID;
  LVLGen() : MachineFunctionPass(ID) {}
  bool runOnMachineBasicBlock(MachineBasicBlock &MBB);
  bool runOnMachineFunction(MachineFunction &F) override;

  unsigned getVL(const MachineInstr &MI);
  int getVLIndex(unsigned Opcode);
};
char LVLGen::ID = 0;

} // end of anonymous namespace

FunctionPass *toolchain::createLVLGenPass() { return new LVLGen; }

int LVLGen::getVLIndex(unsigned Opcode) {
  const MCInstrDesc &MCID = TII->get(Opcode);

  // If an instruction has VLIndex information, return it.
  if (HAS_VLINDEX(MCID.TSFlags))
    return GET_VLINDEX(MCID.TSFlags);

  return -1;
}

// returns a register holding a vector length. NoRegister is returned when
// this MI does not have a vector length.
unsigned LVLGen::getVL(const MachineInstr &MI) {
  int Index = getVLIndex(MI.getOpcode());
  if (Index >= 0)
    return MI.getOperand(Index).getReg();

  return VE::NoRegister;
}

bool LVLGen::runOnMachineBasicBlock(MachineBasicBlock &MBB) {
#define RegName(no)                                                            \
  (MBB.getParent()->getSubtarget<VESubtarget>().getRegisterInfo()->getName(no))

  bool Changed = false;
  bool HasRegForVL = false;
  unsigned RegForVL;

  for (MachineBasicBlock::iterator I = MBB.begin(); I != MBB.end();) {
    MachineBasicBlock::iterator MI = I;

    // Check whether MI uses a vector length operand.  If so, we prepare for VL
    // register.  We would like to reuse VL register as much as possible.  We
    // also would like to keep the number of LEA instructions as fewer as
    // possible.  Therefore, we use a regular scalar register to hold immediate
    // values to load VL register.  And try to reuse identical scalar registers
    // to avoid new LVLr instructions as much as possible.
    unsigned Reg = getVL(*MI);
    if (Reg != VE::NoRegister) {
      LLVM_DEBUG(dbgs() << "Vector instruction found: ");
      LLVM_DEBUG(MI->dump());
      LLVM_DEBUG(dbgs() << "Vector length is " << RegName(Reg) << ". ");
      LLVM_DEBUG(dbgs() << "Current VL is "
                        << (HasRegForVL ? RegName(RegForVL) : "unknown")
                        << ". ");

      if (!HasRegForVL || RegForVL != Reg) {
        // Use VL, but a different value in a different scalar register.
        // So, generate new LVL instruction just before the current instruction.
        LLVM_DEBUG(dbgs() << "Generate a LVL instruction to load "
                          << RegName(Reg) << ".\n");
        BuildMI(MBB, I, MI->getDebugLoc(), TII->get(VE::LVLr)).addReg(Reg);
        HasRegForVL = true;
        RegForVL = Reg;
        Changed = true;
      } else {
        LLVM_DEBUG(dbgs() << "Reuse current VL.\n");
      }
    }
    // Check the update of a given scalar register holding an immediate value
    // for VL register.  Also, a call doesn't preserve VL register.
    if (HasRegForVL) {
      if (MI->definesRegister(RegForVL, TRI) ||
          MI->modifiesRegister(RegForVL, TRI) ||
          MI->killsRegister(RegForVL, TRI) || MI->isCall()) {
        // The latest VL is needed to be updated, so disable HasRegForVL.
        LLVM_DEBUG(dbgs() << RegName(RegForVL) << " is needed to be updated: ");
        LLVM_DEBUG(MI->dump());
        HasRegForVL = false;
      }
    }

    ++I;
  }
  return Changed;
}

bool LVLGen::runOnMachineFunction(MachineFunction &F) {
  LLVM_DEBUG(dbgs() << "********** Begin LVLGen **********\n");
  LLVM_DEBUG(dbgs() << "********** Function: " << F.getName() << '\n');
  LLVM_DEBUG(F.dump());

  bool Changed = false;

  const VESubtarget &Subtarget = F.getSubtarget<VESubtarget>();
  TII = Subtarget.getInstrInfo();
  TRI = Subtarget.getRegisterInfo();

  for (MachineBasicBlock &MBB : F)
    Changed |= runOnMachineBasicBlock(MBB);

  if (Changed) {
    LLVM_DEBUG(dbgs() << "\n");
    LLVM_DEBUG(F.dump());
  }
  LLVM_DEBUG(dbgs() << "********** End LVLGen **********\n");
  return Changed;
}
