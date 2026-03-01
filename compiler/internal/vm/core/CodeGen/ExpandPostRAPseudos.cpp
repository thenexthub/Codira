//===-- ExpandPostRAPseudos.cpp - Pseudo instruction expansion pass -------===//
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
// This file defines a pass that expands COPY and SUBREG_TO_REG pseudo
// instructions after register allocation.
//
//===----------------------------------------------------------------------===//

#include "vm/core/CodeGen/ExpandPostRAPseudos.h"
#include "vm/core/CodeGen/MachineDominators.h"
#include "vm/core/CodeGen/MachineFunctionPass.h"
#include "vm/core/CodeGen/MachineInstr.h"
#include "vm/core/CodeGen/MachineLoopInfo.h"
#include "vm/core/CodeGen/Passes.h"
#include "vm/core/CodeGen/TargetInstrInfo.h"
#include "vm/core/CodeGen/TargetRegisterInfo.h"
#include "vm/core/CodeGen/TargetSubtargetInfo.h"
#include "vm/core/InitializePasses.h"
#include "vm/core/Support/Debug.h"
#include "vm/core/Support/raw_ostream.h"

using namespace vm::core;

#define DEBUG_TYPE "postrapseudos"

namespace {
struct ExpandPostRA {
  bool run(MachineFunction &);

private:
  const TargetRegisterInfo *TRI = nullptr;
  const TargetInstrInfo *TII = nullptr;

  bool LowerSubregToReg(MachineInstr *MI);
};

struct ExpandPostRALegacy : public MachineFunctionPass {
  static char ID;
  ExpandPostRALegacy() : MachineFunctionPass(ID) {
    initializeExpandPostRALegacyPass(*PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesCFG();
    AU.addPreservedID(MachineLoopInfoID);
    AU.addPreservedID(MachineDominatorsID);
    MachineFunctionPass::getAnalysisUsage(AU);
  }

  /// runOnMachineFunction - pass entry point
  bool runOnMachineFunction(MachineFunction &) override;
};
} // end anonymous namespace

PreservedAnalyses
ExpandPostRAPseudosPass::run(MachineFunction &MF,
                             MachineFunctionAnalysisManager &MFAM) {
  if (!ExpandPostRA().run(MF))
    return PreservedAnalyses::all();

  return getMachineFunctionPassPreservedAnalyses()
      .preserveSet<CFGAnalyses>()
      .preserve<MachineLoopAnalysis>()
      .preserve<MachineDominatorTreeAnalysis>();
}

char ExpandPostRALegacy::ID = 0;
char &toolchain::ExpandPostRAPseudosID = ExpandPostRALegacy::ID;

INITIALIZE_PASS(ExpandPostRALegacy, DEBUG_TYPE,
                "Post-RA pseudo instruction expansion pass", false, false)

bool ExpandPostRA::LowerSubregToReg(MachineInstr *MI) {
  MachineBasicBlock *MBB = MI->getParent();
  assert((MI->getOperand(0).isReg() && MI->getOperand(0).isDef()) &&
         MI->getOperand(1).isImm() &&
         (MI->getOperand(2).isReg() && MI->getOperand(2).isUse()) &&
          MI->getOperand(3).isImm() && "Invalid subreg_to_reg");

  Register DstReg = MI->getOperand(0).getReg();
  Register InsReg = MI->getOperand(2).getReg();
  assert(!MI->getOperand(2).getSubReg() && "SubIdx on physreg?");
  unsigned SubIdx  = MI->getOperand(3).getImm();

  assert(SubIdx != 0 && "Invalid index for insert_subreg");
  Register DstSubReg = TRI->getSubReg(DstReg, SubIdx);

  assert(DstReg.isPhysical() &&
         "Insert destination must be in a physical register");
  assert(InsReg.isPhysical() &&
         "Inserted value must be in a physical register");

  LLVM_DEBUG(dbgs() << "subreg: CONVERTING: " << *MI);

  if (MI->allDefsAreDead()) {
    MI->setDesc(TII->get(TargetOpcode::KILL));
    MI->removeOperand(3); // SubIdx
    MI->removeOperand(1); // Imm
    LLVM_DEBUG(dbgs() << "subreg: replaced by: " << *MI);
    return true;
  }

  if (DstSubReg == InsReg) {
    // No need to insert an identity copy instruction.
    // Watch out for case like this:
    // %rax = SUBREG_TO_REG 0, killed %eax, 3
    // We must leave %rax live.
    if (DstReg != InsReg) {
      MI->setDesc(TII->get(TargetOpcode::KILL));
      MI->removeOperand(3);     // SubIdx
      MI->removeOperand(1);     // Imm
      LLVM_DEBUG(dbgs() << "subreg: replace by: " << *MI);
      return true;
    }
    LLVM_DEBUG(dbgs() << "subreg: eliminated!");
  } else {
    TII->copyPhysReg(*MBB, MI, MI->getDebugLoc(), DstSubReg, InsReg,
                     MI->getOperand(2).isKill());

    // Implicitly define DstReg for subsequent uses.
    MachineBasicBlock::iterator CopyMI = MI;
    --CopyMI;
    CopyMI->addRegisterDefined(DstReg);
    LLVM_DEBUG(dbgs() << "subreg: " << *CopyMI);
  }

  LLVM_DEBUG(dbgs() << '\n');
  MBB->erase(MI);
  return true;
}

bool ExpandPostRALegacy::runOnMachineFunction(MachineFunction &MF) {
  return ExpandPostRA().run(MF);
}

/// runOnMachineFunction - Reduce subregister inserts and extracts to register
/// copies.
///
bool ExpandPostRA::run(MachineFunction &MF) {
  LLVM_DEBUG(dbgs() << "Machine Function\n"
                    << "********** EXPANDING POST-RA PSEUDO INSTRS **********\n"
                    << "********** Function: " << MF.getName() << '\n');
  TRI = MF.getSubtarget().getRegisterInfo();
  TII = MF.getSubtarget().getInstrInfo();

  bool MadeChange = false;

  for (MachineBasicBlock &MBB : MF) {
    for (MachineInstr &MI : toolchain::make_early_inc_range(MBB)) {
      // Only expand pseudos.
      if (!MI.isPseudo())
        continue;

      // Give targets a chance to expand even standard pseudos.
      if (TII->expandPostRAPseudo(MI)) {
        MadeChange = true;
        continue;
      }

      // Expand standard pseudos.
      switch (MI.getOpcode()) {
      case TargetOpcode::SUBREG_TO_REG:
        MadeChange |= LowerSubregToReg(&MI);
        break;
      case TargetOpcode::COPY:
        TII->lowerCopy(&MI, TRI);
        MadeChange = true;
        break;
      case TargetOpcode::DBG_VALUE:
        continue;
      case TargetOpcode::INSERT_SUBREG:
      case TargetOpcode::EXTRACT_SUBREG:
        llvm_unreachable("Sub-register pseudos should have been eliminated.");
      }
    }
  }

  return MadeChange;
}
