//===- DeadMachineInstructionElim.cpp - Remove dead machine instructions --===//
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
// This is an extremely simple MachineInstr-level dead-code-elimination pass.
//
//===----------------------------------------------------------------------===//

#include "vm/core/CodeGen/DeadMachineInstructionElim.h"
#include "vm/core/ADT/PostOrderIterator.h"
#include "vm/core/ADT/Statistic.h"
#include "vm/core/CodeGen/LiveRegUnits.h"
#include "vm/core/CodeGen/MachineFunctionPass.h"
#include "vm/core/CodeGen/MachineRegisterInfo.h"
#include "vm/core/CodeGen/TargetSubtargetInfo.h"
#include "vm/core/InitializePasses.h"
#include "vm/core/Pass.h"
#include "vm/core/Support/Debug.h"
#include "vm/core/Support/raw_ostream.h"

using namespace vm::core;

#define DEBUG_TYPE "dead-mi-elimination"

STATISTIC(NumDeletes,          "Number of dead instructions deleted");

namespace {
class DeadMachineInstructionElimImpl {
  const MachineRegisterInfo *MRI = nullptr;
  const TargetInstrInfo *TII = nullptr;
  LiveRegUnits LivePhysRegs;

public:
  bool runImpl(MachineFunction &MF);

private:
  bool eliminateDeadMI(MachineFunction &MF);
};

class DeadMachineInstructionElim : public MachineFunctionPass {
public:
  static char ID; // Pass identification, replacement for typeid

  DeadMachineInstructionElim() : MachineFunctionPass(ID) {
    initializeDeadMachineInstructionElimPass(*PassRegistry::getPassRegistry());
  }

  bool runOnMachineFunction(MachineFunction &MF) override {
    if (skipFunction(MF.getFunction()))
      return false;
    return DeadMachineInstructionElimImpl().runImpl(MF);
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesCFG();
    MachineFunctionPass::getAnalysisUsage(AU);
  }
};
} // namespace

PreservedAnalyses
DeadMachineInstructionElimPass::run(MachineFunction &MF,
                                    MachineFunctionAnalysisManager &) {
  if (!DeadMachineInstructionElimImpl().runImpl(MF))
    return PreservedAnalyses::all();
  PreservedAnalyses PA = getMachineFunctionPassPreservedAnalyses();
  PA.preserveSet<CFGAnalyses>();
  return PA;
}

char DeadMachineInstructionElim::ID = 0;
char &toolchain::DeadMachineInstructionElimID = DeadMachineInstructionElim::ID;

INITIALIZE_PASS(DeadMachineInstructionElim, DEBUG_TYPE,
                "Remove dead machine instructions", false, false)

bool DeadMachineInstructionElimImpl::runImpl(MachineFunction &MF) {
  MRI = &MF.getRegInfo();

  const TargetSubtargetInfo &ST = MF.getSubtarget();
  TII = ST.getInstrInfo();
  LivePhysRegs.init(*ST.getRegisterInfo());

  bool AnyChanges = eliminateDeadMI(MF);
  while (AnyChanges && eliminateDeadMI(MF))
    ;
  return AnyChanges;
}

bool DeadMachineInstructionElimImpl::eliminateDeadMI(MachineFunction &MF) {
  bool AnyChanges = false;

  // Loop over all instructions in all blocks, from bottom to top, so that it's
  // more likely that chains of dependent but ultimately dead instructions will
  // be cleaned up.
  for (MachineBasicBlock *MBB : post_order(&MF)) {
    LivePhysRegs.addLiveOuts(*MBB);

    // Now scan the instructions and delete dead ones, tracking physreg
    // liveness as we go.
    for (MachineInstr &MI : make_early_inc_range(reverse(*MBB))) {
      // If the instruction is dead, delete it!
      if (MI.isDead(*MRI, &LivePhysRegs)) {
        LLVM_DEBUG(dbgs() << "DeadMachineInstructionElim: DELETING: " << MI);
        // It is possible that some DBG_VALUE instructions refer to this
        // instruction. They will be deleted in the live debug variable
        // analysis.
        MI.eraseFromParent();
        AnyChanges = true;
        ++NumDeletes;
        continue;
      }
      LivePhysRegs.stepBackward(MI);
    }
  }
  LivePhysRegs.clear();
  return AnyChanges;
}
