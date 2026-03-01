//===-- PPCCTRLoops.cpp - Verify CTR loops -----------------===//
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
// This pass verifies that all bdnz/bdz instructions are dominated by a loop
// mtctr before any other instructions that might clobber the ctr register.
//
//===----------------------------------------------------------------------===//

// CTR loops are produced by the HardwareLoops pass and this pass is simply a
// verification that no invalid CTR loops are produced. As such, it isn't
// something that needs to be run (or even defined) for Release builds so the
// entire file is guarded by NDEBUG.
#ifndef NDEBUG
#include "MCTargetDesc/PPCMCTargetDesc.h"
#include "PPC.h"
#include "vm/core/ADT/SmallVector.h"
#include "vm/core/ADT/StringRef.h"
#include "vm/core/ADT/ilist_iterator.h"
#include "vm/core/CodeGen/MachineBasicBlock.h"
#include "vm/core/CodeGen/MachineDominators.h"
#include "vm/core/CodeGen/MachineFunction.h"
#include "vm/core/CodeGen/MachineFunctionPass.h"
#include "vm/core/CodeGen/MachineInstr.h"
#include "vm/core/CodeGen/MachineInstrBundleIterator.h"
#include "vm/core/CodeGen/MachineOperand.h"
#include "vm/core/CodeGen/Register.h"
#include "vm/core/InitializePasses.h"
#include "vm/core/Pass.h"
#include "vm/core/PassRegistry.h"
#include "vm/core/Support/Debug.h"
#include "vm/core/Support/ErrorHandling.h"
#include "vm/core/Support/Printable.h"
#include "vm/core/Support/raw_ostream.h"

using namespace vm::core;

#define DEBUG_TYPE "ppc-ctrloops-verify"

namespace {

  struct PPCCTRLoopsVerify : public MachineFunctionPass {
  public:
    static char ID;

    PPCCTRLoopsVerify() : MachineFunctionPass(ID) {
      initializePPCCTRLoopsVerifyPass(*PassRegistry::getPassRegistry());
    }

    void getAnalysisUsage(AnalysisUsage &AU) const override {
      AU.addRequired<MachineDominatorTreeWrapperPass>();
      MachineFunctionPass::getAnalysisUsage(AU);
    }

    bool runOnMachineFunction(MachineFunction &MF) override;

  private:
    MachineDominatorTree *MDT;
  };

  char PPCCTRLoopsVerify::ID = 0;
} // end anonymous namespace

INITIALIZE_PASS_BEGIN(PPCCTRLoopsVerify, "ppc-ctr-loops-verify",
                      "PowerPC CTR Loops Verify", false, false)
INITIALIZE_PASS_DEPENDENCY(MachineDominatorTreeWrapperPass)
INITIALIZE_PASS_END(PPCCTRLoopsVerify, "ppc-ctr-loops-verify",
                    "PowerPC CTR Loops Verify", false, false)

FunctionPass *toolchain::createPPCCTRLoopsVerify() {
  return new PPCCTRLoopsVerify();
}

static bool clobbersCTR(const MachineInstr &MI) {
  for (const MachineOperand &MO : MI.operands()) {
    if (MO.isReg()) {
      if (MO.isDef() && (MO.getReg() == PPC::CTR || MO.getReg() == PPC::CTR8))
        return true;
    } else if (MO.isRegMask()) {
      if (MO.clobbersPhysReg(PPC::CTR) || MO.clobbersPhysReg(PPC::CTR8))
        return true;
    }
  }

  return false;
}

static bool verifyCTRBranch(MachineBasicBlock *MBB,
                            MachineBasicBlock::iterator I) {
  MachineBasicBlock::iterator BI = I;
  SmallPtrSet<MachineBasicBlock *, 16> Visited;
  SmallVector<MachineBasicBlock *, 8> Preds;
  bool CheckPreds;

  if (I == MBB->begin()) {
    Visited.insert(MBB);
    goto queue_preds;
  } else
    --I;

check_block:
  Visited.insert(MBB);
  if (I == MBB->end())
    goto queue_preds;

  CheckPreds = true;
  for (MachineBasicBlock::iterator IE = MBB->begin();; --I) {
    unsigned Opc = I->getOpcode();
    if (Opc == PPC::MTCTRloop || Opc == PPC::MTCTR8loop) {
      CheckPreds = false;
      break;
    }

    if (I != BI && clobbersCTR(*I)) {
      LLVM_DEBUG(dbgs() << printMBBReference(*MBB) << " (" << MBB->getFullName()
                        << ") instruction " << *I
                        << " clobbers CTR, invalidating "
                        << printMBBReference(*BI->getParent()) << " ("
                        << BI->getParent()->getFullName() << ") instruction "
                        << *BI << "\n");
      return false;
    }

    if (I == IE)
      break;
  }

  if (!CheckPreds && Preds.empty())
    return true;

  if (CheckPreds) {
queue_preds:
    if (MachineFunction::iterator(MBB) == MBB->getParent()->begin()) {
      LLVM_DEBUG(dbgs() << "Unable to find a MTCTR instruction for "
                        << printMBBReference(*BI->getParent()) << " ("
                        << BI->getParent()->getFullName() << ") instruction "
                        << *BI << "\n");
      return false;
    }

    append_range(Preds, MBB->predecessors());
  }

  do {
    MBB = Preds.pop_back_val();
    if (!Visited.count(MBB)) {
      I = MBB->getLastNonDebugInstr();
      goto check_block;
    }
  } while (!Preds.empty());

  return true;
}

bool PPCCTRLoopsVerify::runOnMachineFunction(MachineFunction &MF) {
  MDT = &getAnalysis<MachineDominatorTreeWrapperPass>().getDomTree();

  // Verify that all bdnz/bdz instructions are dominated by a loop mtctr before
  // any other instructions that might clobber the ctr register.
  for (MachineBasicBlock &MBB : MF) {
    if (!MDT->isReachableFromEntry(&MBB))
      continue;

    for (MachineBasicBlock::iterator MII = MBB.getFirstTerminator(),
      MIIE = MBB.end(); MII != MIIE; ++MII) {
      unsigned Opc = MII->getOpcode();
      if (Opc == PPC::BDNZ8 || Opc == PPC::BDNZ ||
          Opc == PPC::BDZ8  || Opc == PPC::BDZ)
        if (!verifyCTRBranch(&MBB, MII))
          llvm_unreachable("Invalid PPC CTR loop!");
    }
  }

  return false;
}
#endif // NDEBUG
