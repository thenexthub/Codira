//===---- RemoveLoadsIntoFakeUses.cpp - Remove loads with no real uses ----===//
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
/// \file
/// The FAKE_USE instruction is used to preserve certain values through
/// optimizations for the sake of debugging. This may result in spilled values
/// being loaded into registers that are only used by FAKE_USEs; this is not
/// necessary for debugging purposes, because at that point the value must be on
/// the stack and hence available for debugging. Therefore, this pass removes
/// loads that are only used by FAKE_USEs.
///
/// This pass should run very late, to ensure that we don't inadvertently
/// shorten stack lifetimes by removing these loads, since the FAKE_USEs will
/// also no longer be in effect. Running immediately before LiveDebugValues
/// ensures that LDV will have accurate information of the machine location of
/// debug values.
///
//===----------------------------------------------------------------------===//

#include "vm/core/CodeGen/RemoveLoadsIntoFakeUses.h"
#include "vm/core/ADT/PostOrderIterator.h"
#include "vm/core/ADT/Statistic.h"
#include "vm/core/CodeGen/LiveRegUnits.h"
#include "vm/core/CodeGen/MachineFunction.h"
#include "vm/core/CodeGen/MachineFunctionPass.h"
#include "vm/core/CodeGen/MachinePassManager.h"
#include "vm/core/CodeGen/MachineRegisterInfo.h"
#include "vm/core/CodeGen/TargetSubtargetInfo.h"
#include "vm/core/IR/Function.h"
#include "vm/core/InitializePasses.h"
#include "vm/core/Support/Debug.h"
#include "vm/core/Target/TargetMachine.h"

using namespace vm::core;

#define DEBUG_TYPE "remove-loads-into-fake-uses"

STATISTIC(NumLoadsDeleted, "Number of dead load instructions deleted");
STATISTIC(NumFakeUsesDeleted, "Number of FAKE_USE instructions deleted");

class RemoveLoadsIntoFakeUsesLegacy : public MachineFunctionPass {
public:
  static char ID;

  RemoveLoadsIntoFakeUsesLegacy() : MachineFunctionPass(ID) {
    initializeRemoveLoadsIntoFakeUsesLegacyPass(
        *PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesCFG();
    MachineFunctionPass::getAnalysisUsage(AU);
  }

  MachineFunctionProperties getRequiredProperties() const override {
    return MachineFunctionProperties().setNoVRegs();
  }

  StringRef getPassName() const override {
    return "Remove Loads Into Fake Uses";
  }

  bool runOnMachineFunction(MachineFunction &MF) override;
};

struct RemoveLoadsIntoFakeUses {
  bool run(MachineFunction &MF);
};

char RemoveLoadsIntoFakeUsesLegacy::ID = 0;
char &toolchain::RemoveLoadsIntoFakeUsesID = RemoveLoadsIntoFakeUsesLegacy::ID;

INITIALIZE_PASS_BEGIN(RemoveLoadsIntoFakeUsesLegacy, DEBUG_TYPE,
                      "Remove Loads Into Fake Uses", false, false)
INITIALIZE_PASS_END(RemoveLoadsIntoFakeUsesLegacy, DEBUG_TYPE,
                    "Remove Loads Into Fake Uses", false, false)

bool RemoveLoadsIntoFakeUsesLegacy::runOnMachineFunction(MachineFunction &MF) {
  if (skipFunction(MF.getFunction()))
    return false;

  return RemoveLoadsIntoFakeUses().run(MF);
}

PreservedAnalyses
RemoveLoadsIntoFakeUsesPass::run(MachineFunction &MF,
                                 MachineFunctionAnalysisManager &MFAM) {
  MFPropsModifier _(*this, MF);

  if (!RemoveLoadsIntoFakeUses().run(MF))
    return PreservedAnalyses::all();

  auto PA = getMachineFunctionPassPreservedAnalyses();
  PA.preserveSet<CFGAnalyses>();
  return PA;
}

bool RemoveLoadsIntoFakeUses::run(MachineFunction &MF) {
  // Skip this pass if we would use VarLoc-based LDV, as there may be DBG_VALUE
  // instructions of the restored values that would become invalid.
  if (!MF.useDebugInstrRef())
    return false;
  // Only run this for functions that have fake uses.
  if (!MF.hasFakeUses())
    return false;

  bool AnyChanges = false;

  LiveRegUnits LivePhysRegs;
  const MachineRegisterInfo *MRI = &MF.getRegInfo();
  const TargetSubtargetInfo &ST = MF.getSubtarget();
  const TargetInstrInfo *TII = ST.getInstrInfo();
  const TargetRegisterInfo *TRI = ST.getRegisterInfo();

  SmallVector<MachineInstr *> RegFakeUses;
  LivePhysRegs.init(*TRI);
  for (MachineBasicBlock *MBB : post_order(&MF)) {
    RegFakeUses.clear();
    LivePhysRegs.addLiveOuts(*MBB);

    for (MachineInstr &MI : make_early_inc_range(reverse(*MBB))) {
      if (MI.isFakeUse()) {
        if (MI.getNumOperands() == 0 || !MI.getOperand(0).isReg())
          continue;
        // Track the Fake Uses that use these register units so that we can
        // delete them if we delete the corresponding load.
        RegFakeUses.push_back(&MI);
        // Do not record FAKE_USE uses in LivePhysRegs so that we can recognize
        // otherwise-unused loads.
        continue;
      }

      // If the restore size is not std::nullopt then we are dealing with a
      // reload of a spilled register.
      if (MI.getRestoreSize(TII)) {
        Register Reg = MI.getOperand(0).getReg();
        // Don't delete live physreg defs, or any reserved register defs.
        if (!LivePhysRegs.available(Reg) || MRI->isReserved(Reg))
          continue;
        // There should typically be an exact match between the loaded register
        // and the FAKE_USE, but sometimes regalloc will choose to load a larger
        // value than is needed. Therefore, as long as the load isn't used by
        // anything except at least one FAKE_USE, we will delete it. If it isn't
        // used by any fake uses, it should still be safe to delete but we
        // choose to ignore it so that this pass has no side effects unrelated
        // to fake uses.
        SmallDenseSet<MachineInstr *> FakeUsesToDelete;
        for (MachineInstr *&FakeUse : reverse(RegFakeUses)) {
          if (FakeUse->readsRegister(Reg, TRI)) {
            FakeUsesToDelete.insert(FakeUse);
            RegFakeUses.erase(&FakeUse);
          }
        }
        if (!FakeUsesToDelete.empty()) {
          LLVM_DEBUG(dbgs() << "RemoveLoadsIntoFakeUses: DELETING: " << MI);
          // Since this load only exists to restore a spilled register and we
          // haven't, run LiveDebugValues yet, there shouldn't be any DBG_VALUEs
          // for this load; otherwise, deleting this would be incorrect.
          MI.eraseFromParent();
          AnyChanges = true;
          ++NumLoadsDeleted;
          for (MachineInstr *FakeUse : FakeUsesToDelete) {
            LLVM_DEBUG(dbgs()
                       << "RemoveLoadsIntoFakeUses: DELETING: " << *FakeUse);
            FakeUse->eraseFromParent();
          }
          NumFakeUsesDeleted += FakeUsesToDelete.size();
        }
        continue;
      }

      // In addition to tracking LivePhysRegs, we need to clear RegFakeUses each
      // time a register is defined, as existing FAKE_USEs no longer apply to
      // that register.
      if (!RegFakeUses.empty()) {
        for (const MachineOperand &MO : MI.operands()) {
          if (!MO.isReg())
            continue;
          Register Reg = MO.getReg();
          // We clear RegFakeUses for this register and all subregisters,
          // because any such FAKE_USE encountered prior is no longer relevant
          // for later encountered loads.
          for (MachineInstr *&FakeUse : reverse(RegFakeUses))
            if (FakeUse->readsRegister(Reg, TRI))
              RegFakeUses.erase(&FakeUse);
        }
      }
      LivePhysRegs.stepBackward(MI);
    }
  }

  return AnyChanges;
}
