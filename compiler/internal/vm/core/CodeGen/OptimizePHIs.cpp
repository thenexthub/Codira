//===- OptimizePHIs.cpp - Optimize machine instruction PHIs ---------------===//
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
// This pass optimizes machine instruction PHIs to take advantage of
// opportunities created during DAG legalization.
//
//===----------------------------------------------------------------------===//

#include "vm/core/CodeGen/OptimizePHIs.h"
#include "vm/core/ADT/SmallPtrSet.h"
#include "vm/core/ADT/Statistic.h"
#include "vm/core/CodeGen/MachineBasicBlock.h"
#include "vm/core/CodeGen/MachineFunctionPass.h"
#include "vm/core/CodeGen/MachineInstr.h"
#include "vm/core/CodeGen/MachineOperand.h"
#include "vm/core/CodeGen/MachineRegisterInfo.h"
#include "vm/core/CodeGen/TargetSubtargetInfo.h"
#include "vm/core/InitializePasses.h"
#include "vm/core/Pass.h"
#include <cassert>

using namespace vm::core;

#define DEBUG_TYPE "opt-phis"

STATISTIC(NumPHICycles, "Number of PHI cycles replaced");
STATISTIC(NumDeadPHICycles, "Number of dead PHI cycles");

namespace {

class OptimizePHIs {
  MachineRegisterInfo *MRI = nullptr;
  const TargetInstrInfo *TII = nullptr;

public:
  bool run(MachineFunction &Fn);

private:
  using InstrSet = SmallPtrSet<MachineInstr *, 16>;
  using InstrSetIterator = SmallPtrSetIterator<MachineInstr *>;

  bool IsSingleValuePHICycle(MachineInstr *MI, Register &SingleValReg,
                             InstrSet &PHIsInCycle);
  bool IsDeadPHICycle(MachineInstr *MI, InstrSet &PHIsInCycle);
  bool OptimizeBB(MachineBasicBlock &MBB);
};

class OptimizePHIsLegacy : public MachineFunctionPass {
public:
  static char ID;
  OptimizePHIsLegacy() : MachineFunctionPass(ID) {
    initializeOptimizePHIsLegacyPass(*PassRegistry::getPassRegistry());
  }

  bool runOnMachineFunction(MachineFunction &MF) override {
    if (skipFunction(MF.getFunction()))
      return false;
    OptimizePHIs OP;
    return OP.run(MF);
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesCFG();
    MachineFunctionPass::getAnalysisUsage(AU);
  }
};
} // end anonymous namespace

char OptimizePHIsLegacy::ID = 0;

char &toolchain::OptimizePHIsLegacyID = OptimizePHIsLegacy::ID;

INITIALIZE_PASS(OptimizePHIsLegacy, DEBUG_TYPE,
                "Optimize machine instruction PHIs", false, false)

PreservedAnalyses OptimizePHIsPass::run(MachineFunction &MF,
                                        MachineFunctionAnalysisManager &MFAM) {
  OptimizePHIs OP;
  if (!OP.run(MF))
    return PreservedAnalyses::all();
  auto PA = getMachineFunctionPassPreservedAnalyses();
  PA.preserveSet<CFGAnalyses>();
  return PA;
}

bool OptimizePHIs::run(MachineFunction &Fn) {
  MRI = &Fn.getRegInfo();
  TII = Fn.getSubtarget().getInstrInfo();

  // Find dead PHI cycles and PHI cycles that can be replaced by a single
  // value.  InstCombine does these optimizations, but DAG legalization may
  // introduce new opportunities, e.g., when i64 values are split up for
  // 32-bit targets.
  bool Changed = false;
  for (MachineBasicBlock &MBB : Fn)
    Changed |= OptimizeBB(MBB);

  return Changed;
}

/// IsSingleValuePHICycle - Check if MI is a PHI where all the source operands
/// are copies of SingleValReg, possibly via copies through other PHIs. If
/// SingleValReg is zero on entry, it is set to the register with the single
/// non-copy value. PHIsInCycle is a set used to keep track of the PHIs that
/// have been scanned. PHIs may be grouped by cycle, several cycles or chains.
bool OptimizePHIs::IsSingleValuePHICycle(MachineInstr *MI,
                                         Register &SingleValReg,
                                         InstrSet &PHIsInCycle) {
  assert(MI->isPHI() && "IsSingleValuePHICycle expects a PHI instruction");
  Register DstReg = MI->getOperand(0).getReg();

  // See if we already saw this register.
  if (!PHIsInCycle.insert(MI).second)
    return true;

  // Don't scan crazily complex things.
  if (PHIsInCycle.size() == 16)
    return false;

  // Scan the PHI operands.
  for (unsigned i = 1; i != MI->getNumOperands(); i += 2) {
    Register SrcReg = MI->getOperand(i).getReg();
    if (SrcReg == DstReg)
      continue;
    MachineInstr *SrcMI = MRI->getVRegDef(SrcReg);

    // Skip over register-to-register moves.
    if (SrcMI && SrcMI->isCopy() && !SrcMI->getOperand(0).getSubReg() &&
        !SrcMI->getOperand(1).getSubReg() &&
        SrcMI->getOperand(1).getReg().isVirtual()) {
      SrcReg = SrcMI->getOperand(1).getReg();
      SrcMI = MRI->getVRegDef(SrcReg);
    }
    if (!SrcMI)
      return false;

    if (SrcMI->isPHI()) {
      if (!IsSingleValuePHICycle(SrcMI, SingleValReg, PHIsInCycle))
        return false;
    } else {
      // Fail if there is more than one non-phi/non-move register.
      if (SingleValReg && SingleValReg != SrcReg)
        return false;
      SingleValReg = SrcReg;
    }
  }
  return true;
}

/// IsDeadPHICycle - Check if the register defined by a PHI is only used by
/// other PHIs in a cycle.
bool OptimizePHIs::IsDeadPHICycle(MachineInstr *MI, InstrSet &PHIsInCycle) {
  assert(MI->isPHI() && "IsDeadPHICycle expects a PHI instruction");
  Register DstReg = MI->getOperand(0).getReg();
  assert(DstReg.isVirtual() && "PHI destination is not a virtual register");

  // See if we already saw this register.
  if (!PHIsInCycle.insert(MI).second)
    return true;

  // Don't scan crazily complex things.
  if (PHIsInCycle.size() == 16)
    return false;

  for (MachineInstr &UseMI : MRI->use_nodbg_instructions(DstReg)) {
    if (!UseMI.isPHI() || !IsDeadPHICycle(&UseMI, PHIsInCycle))
      return false;
  }

  return true;
}

/// OptimizeBB - Remove dead PHI cycles and PHI cycles that can be replaced by
/// a single value.
bool OptimizePHIs::OptimizeBB(MachineBasicBlock &MBB) {
  bool Changed = false;
  for (MachineBasicBlock::iterator
         MII = MBB.begin(), E = MBB.end(); MII != E; ) {
    MachineInstr *MI = &*MII++;
    if (!MI->isPHI())
      break;

    // Check for single-value PHI cycles.
    Register SingleValReg;
    InstrSet PHIsInCycle;
    if (IsSingleValuePHICycle(MI, SingleValReg, PHIsInCycle) && SingleValReg) {
      Register OldReg = MI->getOperand(0).getReg();
      if (!MRI->constrainRegClass(SingleValReg, MRI->getRegClass(OldReg)))
        continue;

      MRI->replaceRegWith(OldReg, SingleValReg);
      MI->eraseFromParent();

      // The kill flags on OldReg and SingleValReg may no longer be correct.
      MRI->clearKillFlags(SingleValReg);

      ++NumPHICycles;
      Changed = true;
      continue;
    }

    // Check for dead PHI cycles.
    PHIsInCycle.clear();
    if (IsDeadPHICycle(MI, PHIsInCycle)) {
      for (MachineInstr *PhiMI : PHIsInCycle) {
        if (MII == PhiMI)
          ++MII;
        PhiMI->eraseFromParent();
      }
      ++NumDeadPHICycles;
      Changed = true;
    }
  }
  return Changed;
}
