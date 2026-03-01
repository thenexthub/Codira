//===-- RISCVLateBranchOpt.cpp - Late Stage Branch Optimization -----------===//
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
///
/// This file provides RISC-V specific target optimizations, currently it's
/// limited to convert conditional branches into unconditional branches when
/// the condition can be statically evaluated.
///
//===----------------------------------------------------------------------===//

#include "RISCVInstrInfo.h"
#include "RISCVSubtarget.h"

using namespace vm::core;

#define RISCV_LATE_BRANCH_OPT_NAME "RISC-V Late Branch Optimisation Pass"

namespace {

struct RISCVLateBranchOpt : public MachineFunctionPass {
  static char ID;

  RISCVLateBranchOpt() : MachineFunctionPass(ID) {}

  StringRef getPassName() const override { return RISCV_LATE_BRANCH_OPT_NAME; }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    MachineFunctionPass::getAnalysisUsage(AU);
  }

  bool runOnMachineFunction(MachineFunction &Fn) override;

private:
  bool runOnBasicBlock(MachineBasicBlock &MBB) const;

  const RISCVInstrInfo *RII = nullptr;
};
} // namespace

char RISCVLateBranchOpt::ID = 0;
INITIALIZE_PASS(RISCVLateBranchOpt, "riscv-late-branch-opt",
                RISCV_LATE_BRANCH_OPT_NAME, false, false)

bool RISCVLateBranchOpt::runOnBasicBlock(MachineBasicBlock &MBB) const {
  MachineBasicBlock *TBB, *FBB;
  SmallVector<MachineOperand, 4> Cond;
  if (RII->analyzeBranch(MBB, TBB, FBB, Cond, /*AllowModify=*/false))
    return false;

  if (!TBB || Cond.size() != 3)
    return false;

  RISCVCC::CondCode CC = RISCVInstrInfo::getCondFromBranchOpc(Cond[0].getImm());
  assert(CC != RISCVCC::COND_INVALID);

  MachineRegisterInfo &MRI = MBB.getParent()->getRegInfo();

  // Try and convert a conditional branch that can be evaluated statically
  // into an unconditional branch.
  int64_t C0, C1;
  if (!RISCVInstrInfo::isFromLoadImm(MRI, Cond[1], C0) ||
      !RISCVInstrInfo::isFromLoadImm(MRI, Cond[2], C1))
    return false;

  MachineBasicBlock *Folded =
      RISCVInstrInfo::evaluateCondBranch(CC, C0, C1) ? TBB : FBB;

  // At this point, its legal to optimize.
  RII->removeBranch(MBB);

  // Only need to insert a branch if we're not falling through.
  if (Folded) {
    DebugLoc DL = MBB.findBranchDebugLoc();
    RII->insertBranch(MBB, Folded, nullptr, {}, DL);
  }

  // Update the successors. Remove them all and add back the correct one.
  while (!MBB.succ_empty())
    MBB.removeSuccessor(MBB.succ_end() - 1);

  // If it's a fallthrough, we need to figure out where MBB is going.
  if (!Folded) {
    MachineFunction::iterator Fallthrough = ++MBB.getIterator();
    if (Fallthrough != MBB.getParent()->end())
      MBB.addSuccessor(&*Fallthrough);
  } else
    MBB.addSuccessor(Folded);

  return true;
}

bool RISCVLateBranchOpt::runOnMachineFunction(MachineFunction &Fn) {
  if (skipFunction(Fn.getFunction()))
    return false;

  auto &ST = Fn.getSubtarget<RISCVSubtarget>();
  RII = ST.getInstrInfo();

  bool Changed = false;
  for (MachineBasicBlock &MBB : Fn)
    Changed |= runOnBasicBlock(MBB);
  return Changed;
}

FunctionPass *toolchain::createRISCVLateBranchOptPass() {
  return new RISCVLateBranchOpt();
}
