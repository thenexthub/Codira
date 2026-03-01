//===--------- PPCVSXWACCCopy.cpp - VSX and WACC Copy Legalization --------===//
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
// A pass which deals with the complexity of generating legal VSX register
// copies to/from register classes which partially overlap with the VSX
// register file and combines the wacc/wacc_hi copies when needed.
//
//===----------------------------------------------------------------------===//

#include "PPC.h"
#include "PPCInstrInfo.h"
#include "PPCTargetMachine.h"
#include "vm/core/ADT/STLExtras.h"
#include "vm/core/ADT/Statistic.h"
#include "vm/core/CodeGen/MachineFrameInfo.h"
#include "vm/core/CodeGen/MachineFunctionPass.h"
#include "vm/core/CodeGen/MachineInstrBuilder.h"
#include "vm/core/CodeGen/MachineMemOperand.h"
#include "vm/core/CodeGen/MachineRegisterInfo.h"
#include "vm/core/Support/ErrorHandling.h"

using namespace vm::core;

#define DEBUG_TYPE "ppc-vsx-copy"

namespace {
// PPCVSXWACCCopy pass - For copies between VSX registers and non-VSX registers
// (Altivec and scalar floating-point registers), we need to transform the
// copies into subregister copies with other restrictions.
struct PPCVSXWACCCopy : public MachineFunctionPass {
  static char ID;
  PPCVSXWACCCopy() : MachineFunctionPass(ID) {}

  const TargetInstrInfo *TII;

  bool IsRegInClass(unsigned Reg, const TargetRegisterClass *RC,
                    MachineRegisterInfo &MRI) {
    if (Register::isVirtualRegister(Reg)) {
      return RC->hasSubClassEq(MRI.getRegClass(Reg));
    } else if (RC->contains(Reg)) {
      return true;
    }

    return false;
  }

  bool IsVSReg(unsigned Reg, MachineRegisterInfo &MRI) {
    return IsRegInClass(Reg, &PPC::VSRCRegClass, MRI);
  }

  bool IsVRReg(unsigned Reg, MachineRegisterInfo &MRI) {
    return IsRegInClass(Reg, &PPC::VRRCRegClass, MRI);
  }

  bool IsF8Reg(unsigned Reg, MachineRegisterInfo &MRI) {
    return IsRegInClass(Reg, &PPC::F8RCRegClass, MRI);
  }

  bool IsVSFReg(unsigned Reg, MachineRegisterInfo &MRI) {
    return IsRegInClass(Reg, &PPC::VSFRCRegClass, MRI);
  }

  bool IsVSSReg(unsigned Reg, MachineRegisterInfo &MRI) {
    return IsRegInClass(Reg, &PPC::VSSRCRegClass, MRI);
  }

protected:
  bool processBlock(MachineBasicBlock &MBB) {
    bool Changed = false;

    MachineRegisterInfo &MRI = MBB.getParent()->getRegInfo();
    for (MachineInstr &MI : MBB) {
      if (!MI.isFullCopy())
        continue;

      MachineOperand &DstMO = MI.getOperand(0);
      MachineOperand &SrcMO = MI.getOperand(1);

      if (IsVSReg(DstMO.getReg(), MRI) && !IsVSReg(SrcMO.getReg(), MRI)) {
        // This is a copy *to* a VSX register from a non-VSX register.
        Changed = true;

        const TargetRegisterClass *SrcRC = &PPC::VSLRCRegClass;
        assert((IsF8Reg(SrcMO.getReg(), MRI) || IsVSSReg(SrcMO.getReg(), MRI) ||
                IsVSFReg(SrcMO.getReg(), MRI)) &&
               "Unknown source for a VSX copy");

        Register NewVReg = MRI.createVirtualRegister(SrcRC);
        BuildMI(MBB, MI, MI.getDebugLoc(),
                TII->get(TargetOpcode::SUBREG_TO_REG), NewVReg)
            .addImm(1) // add 1, not 0, because there is no implicit clearing
                       // of the high bits.
            .add(SrcMO)
            .addImm(PPC::sub_64);

        // The source of the original copy is now the new virtual register.
        SrcMO.setReg(NewVReg);
      } else if (!IsVSReg(DstMO.getReg(), MRI) &&
                 IsVSReg(SrcMO.getReg(), MRI)) {
        // This is a copy *from* a VSX register to a non-VSX register.
        Changed = true;

        const TargetRegisterClass *DstRC = &PPC::VSLRCRegClass;
        assert((IsF8Reg(DstMO.getReg(), MRI) || IsVSFReg(DstMO.getReg(), MRI) ||
                IsVSSReg(DstMO.getReg(), MRI)) &&
               "Unknown destination for a VSX copy");

        // Copy the VSX value into a new VSX register of the correct subclass.
        Register NewVReg = MRI.createVirtualRegister(DstRC);
        BuildMI(MBB, MI, MI.getDebugLoc(), TII->get(TargetOpcode::COPY),
                NewVReg)
            .add(SrcMO);

        // Transform the original copy into a subregister extraction copy.
        SrcMO.setReg(NewVReg);
        SrcMO.setSubReg(PPC::sub_64);
      } else if (IsRegInClass(DstMO.getReg(), &PPC::WACC_HIRCRegClass, MRI) &&
                 IsRegInClass(SrcMO.getReg(), &PPC::WACCRCRegClass, MRI)) {
        // Matches the pattern:
        //   %a:waccrc = COPY %b.sub_wacc_hi:dmrrc
        //   %c:wacc_hirc = COPY %a:waccrc
        // And replaces it with:
        //   %c:wacc_hirc = COPY %b.sub_wacc_hi:dmrrc
        MachineInstr *DefMI = MRI.getUniqueVRegDef(SrcMO.getReg());
        if (!DefMI || !DefMI->isCopy())
          continue;

        MachineOperand &OrigSrc = DefMI->getOperand(1);

        if (!IsRegInClass(OrigSrc.getReg(), &PPC::DMRRCRegClass, MRI))
          continue;

        if (OrigSrc.getSubReg() != PPC::sub_wacc_hi)
          continue;

        // Rewrite the second copy to use the original register's subreg
        SrcMO.setReg(OrigSrc.getReg());
        SrcMO.setSubReg(PPC::sub_wacc_hi);
        Changed = true;

        // Remove the intermediate copy if safe
        if (MRI.use_nodbg_empty(DefMI->getOperand(0).getReg()))
          DefMI->eraseFromParent();
      }
    }

    return Changed;
  }

public:
  bool runOnMachineFunction(MachineFunction &MF) override {
    // If we don't have VSX on the subtarget, don't do anything.
    const PPCSubtarget &STI = MF.getSubtarget<PPCSubtarget>();
    if (!STI.hasVSX())
      return false;
    TII = STI.getInstrInfo();

    bool Changed = false;

    for (MachineBasicBlock &B : toolchain::make_early_inc_range(MF))
      if (processBlock(B))
        Changed = true;

    return Changed;
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    MachineFunctionPass::getAnalysisUsage(AU);
  }
};
} // end anonymous namespace

INITIALIZE_PASS(PPCVSXWACCCopy, DEBUG_TYPE, "PowerPC VSX Copy Legalization",
                false, false)

char PPCVSXWACCCopy::ID = 0;
FunctionPass *toolchain::createPPCVSXWACCCopyPass() { return new PPCVSXWACCCopy(); }
