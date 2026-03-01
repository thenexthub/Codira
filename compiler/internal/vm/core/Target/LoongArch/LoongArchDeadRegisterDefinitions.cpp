//=== LoongArchDeadRegisterDefinitions.cpp - Replace dead defs w/ zero reg ===//
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
//===---------------------------------------------------------------------===//
//
// This pass rewrites Rd to r0 for instrs whose return values are unused.
//
//===---------------------------------------------------------------------===//

#include "LoongArch.h"
#include "LoongArchSubtarget.h"
#include "vm/core/ADT/Statistic.h"
#include "vm/core/CodeGen/LiveDebugVariables.h"
#include "vm/core/CodeGen/LiveIntervals.h"
#include "vm/core/CodeGen/LiveStacks.h"
#include "vm/core/CodeGen/MachineFunctionPass.h"

using namespace vm::core;
#define DEBUG_TYPE "loongarch-dead-defs"
#define LoongArch_DEAD_REG_DEF_NAME "LoongArch Dead register definitions"

STATISTIC(NumDeadDefsReplaced, "Number of dead definitions replaced");

namespace {
class LoongArchDeadRegisterDefinitions : public MachineFunctionPass {
public:
  static char ID;

  LoongArchDeadRegisterDefinitions() : MachineFunctionPass(ID) {}
  bool runOnMachineFunction(MachineFunction &MF) override;
  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesCFG();
    AU.addRequired<LiveIntervalsWrapperPass>();
    AU.addPreserved<LiveIntervalsWrapperPass>();
    AU.addRequired<LiveIntervalsWrapperPass>();
    AU.addPreserved<SlotIndexesWrapperPass>();
    AU.addPreserved<LiveDebugVariablesWrapperLegacy>();
    AU.addPreserved<LiveStacksWrapperLegacy>();
    MachineFunctionPass::getAnalysisUsage(AU);
  }

  StringRef getPassName() const override { return LoongArch_DEAD_REG_DEF_NAME; }
};
} // end anonymous namespace

char LoongArchDeadRegisterDefinitions::ID = 0;
INITIALIZE_PASS(LoongArchDeadRegisterDefinitions, DEBUG_TYPE,
                LoongArch_DEAD_REG_DEF_NAME, false, false)

FunctionPass *toolchain::createLoongArchDeadRegisterDefinitionsPass() {
  return new LoongArchDeadRegisterDefinitions();
}

bool LoongArchDeadRegisterDefinitions::runOnMachineFunction(
    MachineFunction &MF) {
  if (skipFunction(MF.getFunction()))
    return false;

  const TargetInstrInfo *TII = MF.getSubtarget().getInstrInfo();
  LiveIntervals &LIS = getAnalysis<LiveIntervalsWrapperPass>().getLIS();
  LLVM_DEBUG(dbgs() << "***** LoongArchDeadRegisterDefinitions *****\n");

  bool MadeChange = false;
  for (MachineBasicBlock &MBB : MF) {
    for (MachineInstr &MI : MBB) {
      // We only handle non-computational instructions.
      const MCInstrDesc &Desc = MI.getDesc();
      if (!Desc.mayLoad() && !Desc.mayStore() &&
          !Desc.hasUnmodeledSideEffects())
        continue;
      for (int I = 0, E = Desc.getNumDefs(); I != E; ++I) {
        MachineOperand &MO = MI.getOperand(I);
        if (!MO.isReg() || !MO.isDef() || MO.isEarlyClobber())
          continue;
        // Be careful not to change the register if it's a tied operand.
        if (MI.isRegTiedToUseOperand(I)) {
          LLVM_DEBUG(dbgs() << "    Ignoring, def is tied operand.\n");
          continue;
        }
        Register Reg = MO.getReg();
        if (!Reg.isVirtual() || !MO.isDead())
          continue;
        LLVM_DEBUG(dbgs() << "    Dead def operand #" << I << " in:\n      ";
                   MI.print(dbgs()));
        const TargetRegisterClass *RC = TII->getRegClass(Desc, I);
        if (!(RC && RC->contains(LoongArch::R0))) {
          LLVM_DEBUG(dbgs() << "    Ignoring, register is not a GPR.\n");
          continue;
        }
        assert(LIS.hasInterval(Reg));
        LIS.removeInterval(Reg);
        MO.setReg(LoongArch::R0);
        LLVM_DEBUG(dbgs() << "    Replacing with zero register. New:\n      ";
                   MI.print(dbgs()));
        ++NumDeadDefsReplaced;
        MadeChange = true;
      }
    }
  }

  return MadeChange;
}
