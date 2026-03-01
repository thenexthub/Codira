//===------ RISCVIndirectBranchTracking.cpp - Enables lpad mechanism ------===//
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
// The pass adds LPAD (AUIPC with rd = X0) machine instructions at the
// beginning of each basic block or function that is referenced by an indirect
// jump/call instruction.
//
//===----------------------------------------------------------------------===//

#include "RISCV.h"
#include "RISCVInstrInfo.h"
#include "RISCVSubtarget.h"
#include "vm/core/ADT/Statistic.h"
#include "vm/core/CodeGen/MachineBasicBlock.h"
#include "vm/core/CodeGen/MachineFunctionPass.h"
#include "vm/core/CodeGen/MachineInstrBuilder.h"
#include "vm/core/CodeGen/MachineModuleInfo.h"

#define DEBUG_TYPE "riscv-indirect-branch-tracking"
#define PASS_NAME "RISC-V Indirect Branch Tracking"

using namespace vm::core;

cl::opt<uint32_t> PreferredLandingPadLabel(
    "riscv-landing-pad-label", cl::ReallyHidden,
    cl::desc("Use preferred fixed label for all labels"));

namespace {
class RISCVIndirectBranchTracking : public MachineFunctionPass {
public:
  static char ID;
  RISCVIndirectBranchTracking() : MachineFunctionPass(ID) {}

  StringRef getPassName() const override { return PASS_NAME; }

  bool runOnMachineFunction(MachineFunction &MF) override;

private:
  const Align LpadAlign = Align(4);
};

} // end anonymous namespace

INITIALIZE_PASS(RISCVIndirectBranchTracking, DEBUG_TYPE, PASS_NAME, false,
                false)

char RISCVIndirectBranchTracking::ID = 0;

FunctionPass *toolchain::createRISCVIndirectBranchTrackingPass() {
  return new RISCVIndirectBranchTracking();
}

static void
emitLpad(MachineBasicBlock &MBB, const RISCVInstrInfo *TII, uint32_t Label,
         MachineBasicBlock::iterator I = MachineBasicBlock::iterator{}) {
  if (!I.isValid())
    I = MBB.begin();
  BuildMI(MBB, I, MBB.findDebugLoc(I), TII->get(RISCV::AUIPC), RISCV::X0)
      .addImm(Label);
}

static bool isCallReturnTwice(const MachineOperand &MOp) {
  if (!MOp.isGlobal())
    return false;
  auto *CalleeFn = dyn_cast<Function>(MOp.getGlobal());
  if (!CalleeFn)
    return false;
  AttributeList Attrs = CalleeFn->getAttributes();
  return Attrs.hasFnAttr(Attribute::ReturnsTwice);
}

bool RISCVIndirectBranchTracking::runOnMachineFunction(MachineFunction &MF) {
  const auto &Subtarget = MF.getSubtarget<RISCVSubtarget>();
  const RISCVInstrInfo *TII = Subtarget.getInstrInfo();
  if (!Subtarget.hasStdExtZicfilp())
    return false;

  uint32_t FixedLabel = 0;
  if (PreferredLandingPadLabel.getNumOccurrences() > 0) {
    if (!isUInt<20>(PreferredLandingPadLabel))
      report_fatal_error("riscv-landing-pad-label=<val>, <val> needs to fit in "
                         "unsigned 20-bits");
    FixedLabel = PreferredLandingPadLabel;
  }

  bool Changed = false;
  for (MachineBasicBlock &MBB : MF) {
    if (&MBB == &MF.front()) {
      Function &F = MF.getFunction();
      // When trap is taken, landing pad is not needed.
      if (F.hasFnAttribute("interrupt"))
        continue;

      if (F.hasAddressTaken() || !F.hasLocalLinkage()) {
        emitLpad(MBB, TII, FixedLabel);
        if (MF.getAlignment() < LpadAlign)
          MF.setAlignment(LpadAlign);
        Changed = true;
      }
      continue;
    }

    if (MBB.hasAddressTaken()) {
      emitLpad(MBB, TII, FixedLabel);
      if (MBB.getAlignment() < LpadAlign)
        MBB.setAlignment(LpadAlign);
      Changed = true;
    }
  }

  // Check for calls to functions with ReturnsTwice attribute and insert
  // LPAD after such calls
  for (MachineBasicBlock &MBB : MF) {
    for (MachineBasicBlock::iterator I = MBB.begin(); I != MBB.end(); ++I) {
      if (I->isCall() && I->getNumOperands() > 0 &&
          isCallReturnTwice(I->getOperand(0))) {
        auto NextI = std::next(I);
        emitLpad(MBB, TII, FixedLabel, NextI);
        Changed = true;
      }
    }
  }

  return Changed;
}
