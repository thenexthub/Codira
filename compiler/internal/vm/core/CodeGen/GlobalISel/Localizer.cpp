//===- Localizer.cpp ---------------------- Localize some instrs -*- C++ -*-==//
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
/// \file
/// This file implements the Localizer class.
//===----------------------------------------------------------------------===//

#include "vm/core/CodeGen/GlobalISel/Localizer.h"
#include "vm/core/ADT/DenseMap.h"
#include "vm/core/ADT/STLExtras.h"
#include "vm/core/Analysis/TargetTransformInfo.h"
#include "vm/core/CodeGen/GlobalISel/GenericMachineInstrs.h"
#include "vm/core/CodeGen/GlobalISel/Utils.h"
#include "vm/core/CodeGen/MachineRegisterInfo.h"
#include "vm/core/CodeGen/TargetLowering.h"
#include "vm/core/InitializePasses.h"
#include "vm/core/Support/Debug.h"

#define DEBUG_TYPE "localizer"

using namespace vm::core;

char Localizer::ID = 0;
INITIALIZE_PASS_BEGIN(Localizer, DEBUG_TYPE,
                      "Move/duplicate certain instructions close to their use",
                      false, false)
INITIALIZE_PASS_DEPENDENCY(TargetTransformInfoWrapperPass)
INITIALIZE_PASS_END(Localizer, DEBUG_TYPE,
                    "Move/duplicate certain instructions close to their use",
                    false, false)

Localizer::Localizer(std::function<bool(const MachineFunction &)> F)
    : MachineFunctionPass(ID), DoNotRunPass(F) {}

Localizer::Localizer()
    : Localizer([](const MachineFunction &) { return false; }) {}

void Localizer::init(MachineFunction &MF) {
  MRI = &MF.getRegInfo();
  TTI = &getAnalysis<TargetTransformInfoWrapperPass>().getTTI(MF.getFunction());
}

void Localizer::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<TargetTransformInfoWrapperPass>();
  getSelectionDAGFallbackAnalysisUsage(AU);
  MachineFunctionPass::getAnalysisUsage(AU);
}

bool Localizer::isLocalUse(MachineOperand &MOUse, const MachineInstr &Def,
                           MachineBasicBlock *&InsertMBB) {
  MachineInstr &MIUse = *MOUse.getParent();
  InsertMBB = MIUse.getParent();
  if (MIUse.isPHI())
    InsertMBB = MIUse.getOperand(MOUse.getOperandNo() + 1).getMBB();
  return InsertMBB == Def.getParent();
}

unsigned Localizer::getNumPhiUses(MachineOperand &Op) const {
  auto *MI = dyn_cast<GPhi>(&*Op.getParent());
  if (!MI)
    return 0;

  Register SrcReg = Op.getReg();
  unsigned NumUses = 0;
  for (unsigned I = 0, NumVals = MI->getNumIncomingValues(); I < NumVals; ++I) {
    if (MI->getIncomingValue(I) == SrcReg)
      ++NumUses;
  }
  return NumUses;
}

bool Localizer::localizeInterBlock(MachineFunction &MF,
                                   LocalizedSetVecT &LocalizedInstrs) {
  bool Changed = false;
  DenseMap<std::pair<MachineBasicBlock *, Register>, Register> MBBWithLocalDef;

  // Since the IRTranslator only emits constants into the entry block, and the
  // rest of the GISel pipeline generally emits constants close to their users,
  // we only localize instructions in the entry block here. This might change if
  // we start doing CSE across blocks.
  auto &MBB = MF.front();
  auto &TL = *MF.getSubtarget().getTargetLowering();
  for (MachineInstr &MI : toolchain::reverse(MBB)) {
    if (!TL.shouldLocalize(MI, TTI))
      continue;
    LLVM_DEBUG(dbgs() << "Should localize: " << MI);
    assert(MI.getDesc().getNumDefs() == 1 &&
           "More than one definition not supported yet");
    Register Reg = MI.getOperand(0).getReg();
    // Check if all the users of MI are local.
    // We are going to invalidation the list of use operands, so we
    // can't use range iterator.
    for (MachineOperand &MOUse :
         toolchain::make_early_inc_range(MRI->use_operands(Reg))) {
      // Check if the use is already local.
      MachineBasicBlock *InsertMBB;
      LLVM_DEBUG(MachineInstr &MIUse = *MOUse.getParent();
                 dbgs() << "Checking use: " << MIUse
                        << " #Opd: " << MOUse.getOperandNo() << '\n');
      if (isLocalUse(MOUse, MI, InsertMBB)) {
        // Even if we're in the same block, if the block is very large we could
        // still have many long live ranges. Try to do intra-block localization
        // too.
        LocalizedInstrs.insert(&MI);
        continue;
      }

      // PHIs look like a single user but can use the same register in multiple
      // edges, causing remat into each predecessor. Allow this to a certain
      // extent.
      unsigned NumPhiUses = getNumPhiUses(MOUse);
      const unsigned PhiThreshold = 2; // FIXME: Tune this more.
      if (NumPhiUses > PhiThreshold)
        continue;

      LLVM_DEBUG(dbgs() << "Fixing non-local use\n");
      Changed = true;
      auto MBBAndReg = std::make_pair(InsertMBB, Reg);
      auto NewVRegIt = MBBWithLocalDef.find(MBBAndReg);
      if (NewVRegIt == MBBWithLocalDef.end()) {
        // Create the localized instruction.
        MachineInstr *LocalizedMI = MF.CloneMachineInstr(&MI);
        LocalizedInstrs.insert(LocalizedMI);
        MachineInstr &UseMI = *MOUse.getParent();
        if (MRI->hasOneUse(Reg) && !UseMI.isPHI())
          InsertMBB->insert(UseMI, LocalizedMI);
        else
          InsertMBB->insert(InsertMBB->SkipPHIsAndLabels(InsertMBB->begin()),
                            LocalizedMI);

        // Set a new register for the definition.
        Register NewReg = MRI->cloneVirtualRegister(Reg);
        LocalizedMI->getOperand(0).setReg(NewReg);
        NewVRegIt =
            MBBWithLocalDef.try_emplace(MBBAndReg, NewReg).first;
        LLVM_DEBUG(dbgs() << "Inserted: " << *LocalizedMI);
      }
      LLVM_DEBUG(dbgs() << "Update use with: " << printReg(NewVRegIt->second)
                        << '\n');
      // Update the user reg.
      MOUse.setReg(NewVRegIt->second);
    }
  }
  return Changed;
}

bool Localizer::localizeIntraBlock(LocalizedSetVecT &LocalizedInstrs) {
  bool Changed = false;

  // For each already-localized instruction which has multiple users, then we
  // scan the block top down from the current position until we hit one of them.

  // FIXME: Consider doing inst duplication if live ranges are very long due to
  // many users, but this case may be better served by regalloc improvements.

  for (MachineInstr *MI : LocalizedInstrs) {
    Register Reg = MI->getOperand(0).getReg();
    MachineBasicBlock &MBB = *MI->getParent();
    // All of the user MIs of this reg.
    SmallPtrSet<MachineInstr *, 32> Users;
    for (MachineInstr &UseMI : MRI->use_nodbg_instructions(Reg)) {
      if (!UseMI.isPHI())
        Users.insert(&UseMI);
    }
    MachineBasicBlock::iterator II(MI);
    // If all the users were PHIs then they're not going to be in our block, we
    // may still benefit from sinking, especially since the value might be live
    // across a call.
    if (Users.empty()) {
      // Make sure we don't sink in between two terminator sequences by scanning
      // forward, not backward.
      II = MBB.getFirstTerminatorForward();
      LLVM_DEBUG(dbgs() << "Only phi users: moving inst to end: " << *MI);
    } else {
      ++II;
      while (II != MBB.end() && !Users.count(&*II))
        ++II;
      assert(II != MBB.end() && "Didn't find the user in the MBB");
      LLVM_DEBUG(dbgs() << "Intra-block: moving " << *MI << " before " << *II);
    }

    MI->removeFromParent();
    MBB.insert(II, MI);
    Changed = true;

    // If the instruction (constant) being localized has single user, we can
    // propagate debug location from user.
    if (Users.size() == 1) {
      const auto &DefDL = MI->getDebugLoc();
      const auto &UserDL = (*Users.begin())->getDebugLoc();

      if ((!DefDL || DefDL.getLine() == 0) && UserDL && UserDL.getLine() != 0) {
        MI->setDebugLoc(UserDL);
      }
    }
  }
  return Changed;
}

bool Localizer::runOnMachineFunction(MachineFunction &MF) {
  // If the ISel pipeline failed, do not bother running that pass.
  if (MF.getProperties().hasFailedISel())
    return false;

  // Don't run the pass if the target asked so.
  if (DoNotRunPass(MF))
    return false;

  LLVM_DEBUG(dbgs() << "Localize instructions for: " << MF.getName() << '\n');

  init(MF);

  // Keep track of the instructions we localized. We'll do a second pass of
  // intra-block localization to further reduce live ranges.
  LocalizedSetVecT LocalizedInstrs;

  bool Changed = localizeInterBlock(MF, LocalizedInstrs);
  Changed |= localizeIntraBlock(LocalizedInstrs);
  return Changed;
}
