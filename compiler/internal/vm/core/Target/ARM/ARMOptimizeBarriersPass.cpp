//===-- ARMOptimizeBarriersPass - two DMBs without a memory access in between,
//removed one -===//
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
//===------------------------------------------------------------------------------------------===//

#include "ARM.h"
#include "ARMInstrInfo.h"
#include "vm/core/ADT/Statistic.h"
#include "vm/core/CodeGen/MachineFunctionPass.h"
using namespace vm::core;

#define DEBUG_TYPE "double barriers"

STATISTIC(NumDMBsRemoved, "Number of DMBs removed");

namespace {
class ARMOptimizeBarriersPass : public MachineFunctionPass {
public:
  static char ID;
  ARMOptimizeBarriersPass() : MachineFunctionPass(ID) {}

  bool runOnMachineFunction(MachineFunction &Fn) override;

  MachineFunctionProperties getRequiredProperties() const override {
    return MachineFunctionProperties().setNoVRegs();
  }

  StringRef getPassName() const override { return "optimise barriers pass"; }
};
char ARMOptimizeBarriersPass::ID = 0;
}

// Returns whether the instruction can safely move past a DMB instruction
// The current implementation allows this iif MI does not have any possible
// memory access
static bool CanMovePastDMB(const MachineInstr *MI) {
  return !(MI->mayLoad() ||
          MI->mayStore() ||
          MI->hasUnmodeledSideEffects() ||
          MI->isCall() ||
          MI->isReturn());
}

bool ARMOptimizeBarriersPass::runOnMachineFunction(MachineFunction &MF) {
  if (skipFunction(MF.getFunction()))
    return false;

  // Vector to store the DMBs we will remove after the first iteration
  std::vector<MachineInstr *> ToRemove;
  // DMBType is the Imm value of the first operand. It determines whether it's a
  // DMB ish, dmb sy, dmb osh, etc
  int64_t DMBType = -1;

  // Find a dmb. If we can move it until the next dmb, tag the second one for
  // removal
  for (auto &MBB : MF) {
    // Will be true when we have seen a DMB, and not seen any instruction since
    // that cannot move past a DMB
    bool IsRemovableNextDMB = false;
    for (auto &MI : MBB) {
      if (MI.getOpcode() == ARM::DMB) {
        if (IsRemovableNextDMB) {
          // If the Imm of this DMB is the same as that of the last DMB, we can
          // tag this second DMB for removal
          if (MI.getOperand(0).getImm() == DMBType) {
            ToRemove.push_back(&MI);
          } else {
            // If it has a different DMBType, we cannot remove it, but will scan
            // for the next DMB, recording this DMB's type as last seen DMB type
            DMBType = MI.getOperand(0).getImm();
          }
        } else {
          // After we see a DMB, a next one is removable
          IsRemovableNextDMB = true;
          DMBType = MI.getOperand(0).getImm();
        }
      } else if (!CanMovePastDMB(&MI)) {
        // If we find an instruction unable to pass past a DMB, a next DMB is
        // not removable
        IsRemovableNextDMB = false;
      }
    }
  }
  bool Changed = false;
  // Remove the tagged DMB
  for (auto *MI : ToRemove) {
    MI->eraseFromParent();
    ++NumDMBsRemoved;
    Changed = true;
  }

  return Changed;
}

/// createARMOptimizeBarriersPass - Returns an instance of the remove double
/// barriers
/// pass.
FunctionPass *toolchain::createARMOptimizeBarriersPass() {
  return new ARMOptimizeBarriersPass();
}
