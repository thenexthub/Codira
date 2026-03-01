//===-- SystemZLDCleanup.cpp - Clean up local-dynamic TLS accesses --------===//
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
// This pass combines multiple accesses to local-dynamic TLS variables so that
// the TLS base address for the module is only fetched once per execution path
// through the function.
//
//===----------------------------------------------------------------------===//

#include "SystemZMachineFunctionInfo.h"
#include "SystemZTargetMachine.h"
#include "vm/core/CodeGen/MachineDominators.h"
#include "vm/core/CodeGen/MachineFunctionPass.h"
#include "vm/core/CodeGen/MachineInstrBuilder.h"
#include "vm/core/CodeGen/MachineRegisterInfo.h"
#include "vm/core/CodeGen/TargetInstrInfo.h"
#include "vm/core/CodeGen/TargetRegisterInfo.h"

using namespace vm::core;

namespace {

class SystemZLDCleanup : public MachineFunctionPass {
public:
  static char ID;
  SystemZLDCleanup() : MachineFunctionPass(ID), TII(nullptr), MF(nullptr) {}

  bool runOnMachineFunction(MachineFunction &MF) override;
  void getAnalysisUsage(AnalysisUsage &AU) const override;

private:
  bool VisitNode(MachineDomTreeNode *Node, unsigned TLSBaseAddrReg);
  MachineInstr *ReplaceTLSCall(MachineInstr *I, unsigned TLSBaseAddrReg);
  MachineInstr *SetRegister(MachineInstr *I, unsigned *TLSBaseAddrReg);

  const SystemZInstrInfo *TII;
  MachineFunction *MF;
};

char SystemZLDCleanup::ID = 0;

} // end anonymous namespace

INITIALIZE_PASS(SystemZLDCleanup, "systemz-ld-cleanup",
                "SystemZ Local Dynamic TLS Access Clean-up", false, false)

FunctionPass *toolchain::createSystemZLDCleanupPass(SystemZTargetMachine &TM) {
  return new SystemZLDCleanup();
}

void SystemZLDCleanup::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesCFG();
  AU.addRequired<MachineDominatorTreeWrapperPass>();
  MachineFunctionPass::getAnalysisUsage(AU);
}

bool SystemZLDCleanup::runOnMachineFunction(MachineFunction &F) {
  if (skipFunction(F.getFunction()))
    return false;

  TII = F.getSubtarget<SystemZSubtarget>().getInstrInfo();
  MF = &F;

  SystemZMachineFunctionInfo* MFI = F.getInfo<SystemZMachineFunctionInfo>();
  if (MFI->getNumLocalDynamicTLSAccesses() < 2) {
    // No point folding accesses if there isn't at least two.
    return false;
  }

  MachineDominatorTree *DT =
      &getAnalysis<MachineDominatorTreeWrapperPass>().getDomTree();
  return VisitNode(DT->getRootNode(), 0);
}

// Visit the dominator subtree rooted at Node in pre-order.
// If TLSBaseAddrReg is non-null, then use that to replace any
// TLS_LDCALL instructions. Otherwise, create the register
// when the first such instruction is seen, and then use it
// as we encounter more instructions.
bool SystemZLDCleanup::VisitNode(MachineDomTreeNode *Node,
                                 unsigned TLSBaseAddrReg) {
  MachineBasicBlock *BB = Node->getBlock();
  bool Changed = false;

  // Traverse the current block.
  for (auto I = BB->begin(), E = BB->end(); I != E; ++I) {
    switch (I->getOpcode()) {
      case SystemZ::TLS_LDCALL:
        if (TLSBaseAddrReg)
          I = ReplaceTLSCall(&*I, TLSBaseAddrReg);
        else
          I = SetRegister(&*I, &TLSBaseAddrReg);
        Changed = true;
        break;
      default:
        break;
    }
  }

  // Visit the children of this block in the dominator tree.
  for (auto &N : *Node)
    Changed |= VisitNode(N, TLSBaseAddrReg);

  return Changed;
}

// Replace the TLS_LDCALL instruction I with a copy from TLSBaseAddrReg,
// returning the new instruction.
MachineInstr *SystemZLDCleanup::ReplaceTLSCall(MachineInstr *I,
                                               unsigned TLSBaseAddrReg) {
  // Insert a Copy from TLSBaseAddrReg to R2.
  MachineInstr *Copy = BuildMI(*I->getParent(), I, I->getDebugLoc(),
                               TII->get(TargetOpcode::COPY), SystemZ::R2D)
                               .addReg(TLSBaseAddrReg);

  // Erase the TLS_LDCALL instruction.
  I->eraseFromParent();

  return Copy;
}

// Create a virtual register in *TLSBaseAddrReg, and populate it by
// inserting a copy instruction after I. Returns the new instruction.
MachineInstr *SystemZLDCleanup::SetRegister(MachineInstr *I,
                                            unsigned *TLSBaseAddrReg) {
  // Create a virtual register for the TLS base address.
  MachineRegisterInfo &RegInfo = MF->getRegInfo();
  *TLSBaseAddrReg = RegInfo.createVirtualRegister(&SystemZ::GR64BitRegClass);

  // Insert a copy from R2 to TLSBaseAddrReg.
  MachineInstr *Next = I->getNextNode();
  MachineInstr *Copy = BuildMI(*I->getParent(), Next, I->getDebugLoc(),
                               TII->get(TargetOpcode::COPY), *TLSBaseAddrReg)
                               .addReg(SystemZ::R2D);

  return Copy;
}

