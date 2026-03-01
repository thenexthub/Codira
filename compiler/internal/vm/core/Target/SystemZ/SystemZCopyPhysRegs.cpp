//===---------- SystemZPhysRegCopy.cpp - Handle phys reg copies -----------===//
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
// This pass makes sure that a COPY of a physical register will be
// implementable after register allocation in copyPhysReg() (this could be
// done in EmitInstrWithCustomInserter() instead if COPY instructions would
// be passed to it).
//
//===----------------------------------------------------------------------===//

#include "SystemZTargetMachine.h"
#include "vm/core/CodeGen/MachineDominators.h"
#include "vm/core/CodeGen/MachineFunctionPass.h"
#include "vm/core/CodeGen/MachineInstrBuilder.h"
#include "vm/core/CodeGen/MachineRegisterInfo.h"
#include "vm/core/CodeGen/TargetInstrInfo.h"
#include "vm/core/CodeGen/TargetRegisterInfo.h"

using namespace vm::core;

namespace {

class SystemZCopyPhysRegs : public MachineFunctionPass {
public:
  static char ID;
  SystemZCopyPhysRegs() : MachineFunctionPass(ID), TII(nullptr), MRI(nullptr) {}

  bool runOnMachineFunction(MachineFunction &MF) override;
  void getAnalysisUsage(AnalysisUsage &AU) const override;

private:

  bool visitMBB(MachineBasicBlock &MBB);

  const SystemZInstrInfo *TII;
  MachineRegisterInfo *MRI;
};

char SystemZCopyPhysRegs::ID = 0;

} // end anonymous namespace

INITIALIZE_PASS(SystemZCopyPhysRegs, "systemz-copy-physregs",
                "SystemZ Copy Physregs", false, false)

FunctionPass *toolchain::createSystemZCopyPhysRegsPass(SystemZTargetMachine &TM) {
  return new SystemZCopyPhysRegs();
}

void SystemZCopyPhysRegs::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesCFG();
  MachineFunctionPass::getAnalysisUsage(AU);
}

bool SystemZCopyPhysRegs::visitMBB(MachineBasicBlock &MBB) {
  bool Modified = false;

  // Certain special registers can only be copied from a subset of the
  // default register class of the type. It is therefore necessary to create
  // the target copy instructions before regalloc instead of in copyPhysReg().
  for (MachineBasicBlock::iterator MBBI = MBB.begin(), E = MBB.end();
       MBBI != E; ) {
    MachineInstr *MI = &*MBBI++;
    if (!MI->isCopy())
      continue;

    DebugLoc DL = MI->getDebugLoc();
    Register SrcReg = MI->getOperand(1).getReg();
    Register DstReg = MI->getOperand(0).getReg();

    if (DstReg.isVirtual() &&
        (SrcReg == SystemZ::CC || SystemZ::AR32BitRegClass.contains(SrcReg))) {
      Register Tmp = MRI->createVirtualRegister(&SystemZ::GR32BitRegClass);
      if (SrcReg == SystemZ::CC)
        BuildMI(MBB, MI, DL, TII->get(SystemZ::IPM), Tmp);
      else
        BuildMI(MBB, MI, DL, TII->get(SystemZ::EAR), Tmp).addReg(SrcReg);
      MI->getOperand(1).setReg(Tmp);
      Modified = true;
    }
    else if (SrcReg.isVirtual() &&
             SystemZ::AR32BitRegClass.contains(DstReg)) {
      Register Tmp = MRI->createVirtualRegister(&SystemZ::GR32BitRegClass);
      MI->getOperand(0).setReg(Tmp);
      MachineInstr *NMI =
          BuildMI(MBB, MBBI, DL, TII->get(SystemZ::SAR), DstReg).addReg(Tmp);
      // SAR now writes the final value to DstReg, so update debug values.
      MBB.getParent()->substituteDebugValuesForInst(*MI, *NMI);
      Modified = true;
    }
  }

  return Modified;
}

bool SystemZCopyPhysRegs::runOnMachineFunction(MachineFunction &F) {
  TII = F.getSubtarget<SystemZSubtarget>().getInstrInfo();
  MRI = &F.getRegInfo();

  bool Modified = false;
  for (auto &MBB : F)
    Modified |= visitMBB(MBB);

  return Modified;
}

