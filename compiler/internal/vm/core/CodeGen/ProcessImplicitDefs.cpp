//===---------------------- ProcessImplicitDefs.cpp -----------------------===//
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

#include "vm/core/CodeGen/ProcessImplicitDefs.h"
#include "vm/core/ADT/SetVector.h"
#include "vm/core/Analysis/AliasAnalysis.h"
#include "vm/core/CodeGen/MachineFunctionPass.h"
#include "vm/core/CodeGen/MachineInstr.h"
#include "vm/core/CodeGen/MachineRegisterInfo.h"
#include "vm/core/CodeGen/TargetInstrInfo.h"
#include "vm/core/CodeGen/TargetSubtargetInfo.h"
#include "vm/core/InitializePasses.h"
#include "vm/core/Pass.h"
#include "vm/core/PassRegistry.h"
#include "vm/core/Support/Debug.h"
#include "vm/core/Support/raw_ostream.h"

using namespace vm::core;

#define DEBUG_TYPE "processimpdefs"

namespace {
/// Process IMPLICIT_DEF instructions and make sure there is one implicit_def
/// for each use. Add isUndef marker to implicit_def defs and their uses.
class ProcessImplicitDefsLegacy : public MachineFunctionPass {
public:
  static char ID;

  ProcessImplicitDefsLegacy() : MachineFunctionPass(ID) {
    initializeProcessImplicitDefsLegacyPass(*PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override;

  bool runOnMachineFunction(MachineFunction &MF) override;

  MachineFunctionProperties getRequiredProperties() const override {
    return MachineFunctionProperties().setIsSSA();
  }
};

class ProcessImplicitDefs {
  const TargetInstrInfo *TII = nullptr;
  const TargetRegisterInfo *TRI = nullptr;
  MachineRegisterInfo *MRI = nullptr;

  SmallSetVector<MachineInstr *, 16> WorkList;

  void processImplicitDef(MachineInstr *MI);
  bool canTurnIntoImplicitDef(MachineInstr *MI);

public:
  bool run(MachineFunction &MF);
};
} // end anonymous namespace

char ProcessImplicitDefsLegacy::ID = 0;
char &toolchain::ProcessImplicitDefsID = ProcessImplicitDefsLegacy::ID;

INITIALIZE_PASS(ProcessImplicitDefsLegacy, DEBUG_TYPE,
                "Process Implicit Definitions", false, false)

void ProcessImplicitDefsLegacy::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesCFG();
  AU.addPreserved<AAResultsWrapperPass>();
  MachineFunctionPass::getAnalysisUsage(AU);
}

bool ProcessImplicitDefs::canTurnIntoImplicitDef(MachineInstr *MI) {
  if (!MI->isCopyLike() &&
      !MI->isInsertSubreg() &&
      !MI->isRegSequence() &&
      !MI->isPHI())
    return false;
  for (const MachineOperand &MO : MI->all_uses())
    if (MO.readsReg())
      return false;
  return true;
}

void ProcessImplicitDefs::processImplicitDef(MachineInstr *MI) {
  LLVM_DEBUG(dbgs() << "Processing " << *MI);
  Register Reg = MI->getOperand(0).getReg();

  if (Reg.isVirtual()) {
    // For virtual registers, mark all uses as <undef>, and convert users to
    // implicit-def when possible.
    for (MachineOperand &MO : MRI->use_nodbg_operands(Reg)) {
      MO.setIsUndef();
      MachineInstr *UserMI = MO.getParent();
      if (!canTurnIntoImplicitDef(UserMI))
        continue;
      LLVM_DEBUG(dbgs() << "Converting to IMPLICIT_DEF: " << *UserMI);
      UserMI->setDesc(TII->get(TargetOpcode::IMPLICIT_DEF));
      WorkList.insert(UserMI);
    }
    MI->eraseFromParent();
    return;
  }

  // This is a physreg implicit-def.
  // Look for the first instruction to use or define an alias.
  MachineBasicBlock::instr_iterator UserMI = MI->getIterator();
  MachineBasicBlock::instr_iterator UserE = MI->getParent()->instr_end();
  bool Found = false;
  for (++UserMI; UserMI != UserE; ++UserMI) {
    for (MachineOperand &MO : UserMI->operands()) {
      if (!MO.isReg())
        continue;
      Register UserReg = MO.getReg();
      if (!UserReg.isPhysical() || !TRI->regsOverlap(Reg, UserReg))
        continue;
      // UserMI uses or redefines Reg. Set <undef> flags on all uses.
      Found = true;
      if (MO.isUse())
        MO.setIsUndef();
    }
    if (Found)
      break;
  }

  // If we found the using MI, we can erase the IMPLICIT_DEF.
  if (Found) {
    LLVM_DEBUG(dbgs() << "Physreg user: " << *UserMI);
    MI->eraseFromParent();
    return;
  }

  // Using instr wasn't found, it could be in another block.
  // Leave the physreg IMPLICIT_DEF, but trim any extra operands.
  for (unsigned i = MI->getNumOperands() - 1; i; --i)
    MI->removeOperand(i);
  LLVM_DEBUG(dbgs() << "Keeping physreg: " << *MI);
}

bool ProcessImplicitDefsLegacy::runOnMachineFunction(MachineFunction &MF) {
  return ProcessImplicitDefs().run(MF);
}

PreservedAnalyses
ProcessImplicitDefsPass::run(MachineFunction &MF,
                             MachineFunctionAnalysisManager &MFAM) {
  if (!ProcessImplicitDefs().run(MF))
    return PreservedAnalyses::all();

  return getMachineFunctionPassPreservedAnalyses()
      .preserveSet<CFGAnalyses>()
      .preserve<AAManager>();
}

/// processImplicitDefs - Process IMPLICIT_DEF instructions and turn them into
/// <undef> operands.
bool ProcessImplicitDefs::run(MachineFunction &MF) {

  LLVM_DEBUG(dbgs() << "********** PROCESS IMPLICIT DEFS **********\n"
                    << "********** Function: " << MF.getName() << '\n');

  bool Changed = false;

  TII = MF.getSubtarget().getInstrInfo();
  TRI = MF.getSubtarget().getRegisterInfo();
  MRI = &MF.getRegInfo();
  assert(WorkList.empty() && "Inconsistent worklist state");

  for (MachineBasicBlock &MBB : MF) {
    // Scan the basic block for implicit defs.
    for (MachineInstr &MI : MBB)
      if (MI.isImplicitDef())
        WorkList.insert(&MI);

    if (WorkList.empty())
      continue;

    LLVM_DEBUG(dbgs() << printMBBReference(MBB) << " has " << WorkList.size()
                      << " implicit defs.\n");
    Changed = true;

    // Drain the WorkList to recursively process any new implicit defs.
    do processImplicitDef(WorkList.pop_back_val());
    while (!WorkList.empty());
  }
  return Changed;
}
