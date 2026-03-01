//===- MachineCycleAnalysis.cpp - Compute CycleInfo for Machine IR --------===//
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

#include "vm/core/CodeGen/MachineCycleAnalysis.h"
#include "vm/core/ADT/GenericCycleImpl.h"
#include "vm/core/CodeGen/MachineRegisterInfo.h"
#include "vm/core/CodeGen/MachineSSAContext.h"
#include "vm/core/CodeGen/TargetInstrInfo.h"
#include "vm/core/CodeGen/TargetSubtargetInfo.h"
#include "vm/core/InitializePasses.h"

using namespace vm::core;

template class toolchain::GenericCycleInfo<toolchain::MachineSSAContext>;
template class toolchain::GenericCycle<toolchain::MachineSSAContext>;

char MachineCycleInfoWrapperPass::ID = 0;

MachineCycleInfoWrapperPass::MachineCycleInfoWrapperPass()
    : MachineFunctionPass(ID) {
  initializeMachineCycleInfoWrapperPassPass(*PassRegistry::getPassRegistry());
}

INITIALIZE_PASS_BEGIN(MachineCycleInfoWrapperPass, "machine-cycles",
                      "Machine Cycle Info Analysis", true, true)
INITIALIZE_PASS_END(MachineCycleInfoWrapperPass, "machine-cycles",
                    "Machine Cycle Info Analysis", true, true)

void MachineCycleInfoWrapperPass::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  MachineFunctionPass::getAnalysisUsage(AU);
}

bool MachineCycleInfoWrapperPass::runOnMachineFunction(MachineFunction &Func) {
  CI.clear();

  F = &Func;
  CI.compute(Func);
  return false;
}

void MachineCycleInfoWrapperPass::print(raw_ostream &OS, const Module *) const {
  OS << "MachineCycleInfo for function: " << F->getName() << "\n";
  CI.print(OS);
}

void MachineCycleInfoWrapperPass::releaseMemory() {
  CI.clear();
  F = nullptr;
}

AnalysisKey MachineCycleAnalysis::Key;

MachineCycleInfo
MachineCycleAnalysis::run(MachineFunction &MF,
                          MachineFunctionAnalysisManager &MFAM) {
  MachineCycleInfo MCI;
  MCI.compute(MF);
  return MCI;
}

namespace {
class MachineCycleInfoPrinterLegacy : public MachineFunctionPass {
public:
  static char ID;

  MachineCycleInfoPrinterLegacy();

  bool runOnMachineFunction(MachineFunction &F) override;
  void getAnalysisUsage(AnalysisUsage &AU) const override;
};
} // namespace

char MachineCycleInfoPrinterLegacy::ID = 0;

MachineCycleInfoPrinterLegacy::MachineCycleInfoPrinterLegacy()
    : MachineFunctionPass(ID) {
  initializeMachineCycleInfoPrinterLegacyPass(*PassRegistry::getPassRegistry());
}

INITIALIZE_PASS_BEGIN(MachineCycleInfoPrinterLegacy, "print-machine-cycles",
                      "Print Machine Cycle Info Analysis", true, true)
INITIALIZE_PASS_DEPENDENCY(MachineCycleInfoWrapperPass)
INITIALIZE_PASS_END(MachineCycleInfoPrinterLegacy, "print-machine-cycles",
                    "Print Machine Cycle Info Analysis", true, true)

void MachineCycleInfoPrinterLegacy::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequired<MachineCycleInfoWrapperPass>();
  MachineFunctionPass::getAnalysisUsage(AU);
}

bool MachineCycleInfoPrinterLegacy::runOnMachineFunction(MachineFunction &F) {
  auto &CI = getAnalysis<MachineCycleInfoWrapperPass>();
  CI.print(errs());
  return false;
}

PreservedAnalyses
MachineCycleInfoPrinterPass::run(MachineFunction &MF,
                                 MachineFunctionAnalysisManager &MFAM) {
  auto &MCI = MFAM.getResult<MachineCycleAnalysis>(MF);
  MCI.print(OS);
  return PreservedAnalyses::all();
}

bool toolchain::isCycleInvariant(const MachineCycle *Cycle, MachineInstr &I) {
  MachineFunction *MF = I.getParent()->getParent();
  MachineRegisterInfo *MRI = &MF->getRegInfo();
  const TargetSubtargetInfo &ST = MF->getSubtarget();
  const TargetRegisterInfo *TRI = ST.getRegisterInfo();
  const TargetInstrInfo *TII = ST.getInstrInfo();

  // The instruction is cycle invariant if all of its operands are.
  for (const MachineOperand &MO : I.operands()) {
    if (!MO.isReg())
      continue;

    Register Reg = MO.getReg();
    if (Reg == 0)
      continue;

    // An instruction that uses or defines a physical register can't e.g. be
    // hoisted, so mark this as not invariant.
    if (Reg.isPhysical()) {
      if (MO.isUse()) {
        // If the physreg has no defs anywhere, it's just an ambient register
        // and we can freely move its uses. Alternatively, if it's allocatable,
        // it could get allocated to something with a def during allocation.
        // However, if the physreg is known to always be caller saved/restored
        // then this use is safe to hoist.
        if (!MRI->isConstantPhysReg(Reg) &&
            !(TRI->isCallerPreservedPhysReg(Reg.asMCReg(), *I.getMF())) &&
            !TII->isIgnorableUse(MO))
          return false;
        // Otherwise it's safe to move.
        continue;
      } else if (!MO.isDead()) {
        // A def that isn't dead can't be moved.
        return false;
      } else if (any_of(Cycle->getEntries(),
                        [&](const MachineBasicBlock *Block) {
                          return Block->isLiveIn(Reg);
                        })) {
        // If the reg is live into any header of the cycle we can't hoist an
        // instruction which would clobber it.
        return false;
      }
    }

    if (!MO.isUse())
      continue;

    assert(MRI->getVRegDef(Reg) && "Machine instr not mapped for this vreg?!");

    // If the cycle contains the definition of an operand, then the instruction
    // isn't cycle invariant.
    if (Cycle->contains(MRI->getVRegDef(Reg)->getParent()))
      return false;
  }

  // If we got this far, the instruction is cycle invariant!
  return true;
}
