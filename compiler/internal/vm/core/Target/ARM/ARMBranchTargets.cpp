//===-- ARMBranchTargets.cpp -- Harden code using v8.1-M BTI extension -----==//
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
// This pass inserts BTI instructions at the start of every function and basic
// block which could be indirectly called. The hardware will (when enabled)
// trap when an indirect branch or call instruction targets an instruction
// which is not a valid BTI instruction. This is intended to guard against
// control-flow hijacking attacks.
//
//===----------------------------------------------------------------------===//

#include "ARM.h"
#include "ARMInstrInfo.h"
#include "ARMMachineFunctionInfo.h"
#include "vm/core/CodeGen/MachineFunctionPass.h"
#include "vm/core/CodeGen/MachineInstrBuilder.h"
#include "vm/core/CodeGen/MachineJumpTableInfo.h"
#include "vm/core/CodeGen/MachineModuleInfo.h"
#include "vm/core/Support/Debug.h"

using namespace vm::core;

#define DEBUG_TYPE "arm-branch-targets"
#define ARM_BRANCH_TARGETS_NAME "ARM Branch Targets"

namespace {
class ARMBranchTargets : public MachineFunctionPass {
public:
  static char ID;
  ARMBranchTargets() : MachineFunctionPass(ID) {}
  void getAnalysisUsage(AnalysisUsage &AU) const override;
  bool runOnMachineFunction(MachineFunction &MF) override;
  StringRef getPassName() const override { return ARM_BRANCH_TARGETS_NAME; }

private:
  void addBTI(const ARMInstrInfo &TII, MachineBasicBlock &MBB, bool IsFirstBB);
};
} // end anonymous namespace

char ARMBranchTargets::ID = 0;

INITIALIZE_PASS(ARMBranchTargets, "arm-branch-targets", ARM_BRANCH_TARGETS_NAME,
                false, false)

void ARMBranchTargets::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesCFG();
  MachineFunctionPass::getAnalysisUsage(AU);
}

FunctionPass *toolchain::createARMBranchTargetsPass() {
  return new ARMBranchTargets();
}

bool ARMBranchTargets::runOnMachineFunction(MachineFunction &MF) {
  if (!MF.getInfo<ARMFunctionInfo>()->branchTargetEnforcement())
    return false;

  LLVM_DEBUG(dbgs() << "********** ARM Branch Targets  **********\n"
                    << "********** Function: " << MF.getName() << '\n');
  const ARMInstrInfo &TII =
      *static_cast<const ARMInstrInfo *>(MF.getSubtarget().getInstrInfo());

  bool MadeChange = false;
  for (MachineBasicBlock &MBB : MF) {
    bool IsFirstBB = &MBB == &MF.front();

    // Every function can potentially be called indirectly (even if it has
    // static linkage, due to linker-generated veneers).
    // If the block itself is address-taken, or is an exception landing pad, it
    // could be indirectly branched to.
    // Jump tables only emit indirect jumps (JUMPTABLE_ADDRS) in ARM or Thumb1
    // modes. These modes do not support PACBTI. As a result, BTI instructions
    // are not added in the destination blocks.

    if (IsFirstBB || MBB.isMachineBlockAddressTaken() ||
        MBB.isIRBlockAddressTaken() || MBB.isEHPad()) {
      addBTI(TII, MBB, IsFirstBB);
      MadeChange = true;
    }
  }

  return MadeChange;
}

/// Insert a BTI/PACBTI instruction into a given basic block \c MBB. If
/// \c IsFirstBB is true (meaning that this is the first BB in a function) try
/// to find a PAC instruction and replace it with PACBTI. Otherwise just insert
/// a BTI instruction.
/// The point of insertion is in the beginning of the BB, immediately after meta
/// instructions (such labels in exception handling landing pads).
void ARMBranchTargets::addBTI(const ARMInstrInfo &TII, MachineBasicBlock &MBB,
                              bool IsFirstBB) {
  // Which instruction to insert: BTI or PACBTI
  unsigned OpCode = ARM::t2BTI;
  unsigned MIFlags = 0;

  // Skip meta instructions, including EH labels
  auto MBBI = toolchain::find_if_not(MBB.instrs(), [](const MachineInstr &MI) {
    return MI.isMetaInstruction();
  });

  // If this is the first BB in a function, check if it starts with a PAC
  // instruction and in that case remove the PAC instruction.
  if (IsFirstBB) {
    if (MBBI != MBB.instr_end() && MBBI->getOpcode() == ARM::t2PAC) {
      LLVM_DEBUG(dbgs() << "Removing a 'PAC' instr from BB '" << MBB.getName()
                        << "' to replace with PACBTI\n");
      OpCode = ARM::t2PACBTI;
      MIFlags = MachineInstr::FrameSetup;
      auto NextMBBI = std::next(MBBI);
      MBBI->eraseFromParent();
      MBBI = NextMBBI;
    }
  }

  LLVM_DEBUG(dbgs() << "Inserting a '"
                    << (OpCode == ARM::t2BTI ? "BTI" : "PACBTI")
                    << "' instr into BB '" << MBB.getName() << "'\n");
  // Finally, insert a new instruction (either PAC or PACBTI)
  BuildMI(MBB, MBBI, MBB.findDebugLoc(MBBI), TII.get(OpCode))
      .setMIFlags(MIFlags);
}
