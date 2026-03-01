//=- MachineLoopUtils.cpp - Functions for manipulating loops ----------------=//
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

#include "vm/core/CodeGen/MachineLoopUtils.h"
#include "vm/core/CodeGen/MachineBasicBlock.h"
#include "vm/core/CodeGen/MachineRegisterInfo.h"
#include "vm/core/CodeGen/TargetInstrInfo.h"
using namespace vm::core;

namespace {
// MI's parent and BB are clones of each other. Find the equivalent copy of MI
// in BB.
MachineInstr &findEquivalentInstruction(MachineInstr &MI,
                                        MachineBasicBlock *BB) {
  MachineBasicBlock *PB = MI.getParent();
  unsigned Offset = std::distance(PB->instr_begin(), MachineBasicBlock::instr_iterator(MI));
  return *std::next(BB->instr_begin(), Offset);
}
} // namespace

MachineBasicBlock *toolchain::PeelSingleBlockLoop(LoopPeelDirection Direction,
                                             MachineBasicBlock *Loop,
                                             MachineRegisterInfo &MRI,
                                             const TargetInstrInfo *TII) {
  MachineFunction &MF = *Loop->getParent();
  MachineBasicBlock *Preheader = *Loop->pred_begin();
  if (Preheader == Loop)
    Preheader = *std::next(Loop->pred_begin());
  MachineBasicBlock *Exit = *Loop->succ_begin();
  if (Exit == Loop)
    Exit = *std::next(Loop->succ_begin());

  MachineBasicBlock *NewBB = MF.CreateMachineBasicBlock(Loop->getBasicBlock());
  if (Direction == LPD_Front)
    MF.insert(Loop->getIterator(), NewBB);
  else
    MF.insert(std::next(Loop->getIterator()), NewBB);

  DenseMap<Register, Register> Remaps;
  auto InsertPt = NewBB->end();
  for (MachineInstr &MI : *Loop) {
    MachineInstr *NewMI = MF.CloneMachineInstr(&MI);
    NewBB->insert(InsertPt, NewMI);
    for (MachineOperand &MO : NewMI->defs()) {
      Register OrigR = MO.getReg();
      if (OrigR.isPhysical())
        continue;
      Register &R = Remaps[OrigR];
      R = MRI.createVirtualRegister(MRI.getRegClass(OrigR));
      MO.setReg(R);

      if (Direction == LPD_Back) {
        // Replace all uses outside the original loop with the new register.
        // FIXME: is the use_iterator stable enough to mutate register uses
        // while iterating?
        SmallVector<MachineOperand *, 4> Uses;
        for (auto &Use : MRI.use_operands(OrigR))
          if (Use.getParent()->getParent() != Loop)
            Uses.push_back(&Use);
        for (auto *Use : Uses) {
          const TargetRegisterClass *ConstrainRegClass =
              MRI.constrainRegClass(R, MRI.getRegClass(Use->getReg()));
          assert(ConstrainRegClass &&
                 "Expected a valid constrained register class!");
          (void)ConstrainRegClass;
          Use->setReg(R);
        }
      }
    }
  }

  for (auto I = NewBB->getFirstNonPHI(); I != NewBB->end(); ++I)
    for (MachineOperand &MO : I->uses())
      if (MO.isReg())
        if (auto It = Remaps.find(MO.getReg()); It != Remaps.end())
          MO.setReg(It->second);

  for (auto I = NewBB->begin(); I->isPHI(); ++I) {
    MachineInstr &MI = *I;
    unsigned LoopRegIdx = 3, InitRegIdx = 1;
    if (MI.getOperand(2).getMBB() != Preheader)
      std::swap(LoopRegIdx, InitRegIdx);
    MachineInstr &OrigPhi = findEquivalentInstruction(MI, Loop);
    assert(OrigPhi.isPHI());
    if (Direction == LPD_Front) {
      // When peeling front, we are only left with the initial value from the
      // preheader.
      Register R = MI.getOperand(LoopRegIdx).getReg();
      if (auto It = Remaps.find(R); It != Remaps.end())
        R = It->second;
      OrigPhi.getOperand(InitRegIdx).setReg(R);
      MI.removeOperand(LoopRegIdx + 1);
      MI.removeOperand(LoopRegIdx + 0);
    } else {
      // When peeling back, the initial value is the loop-carried value from
      // the original loop.
      Register LoopReg = OrigPhi.getOperand(LoopRegIdx).getReg();
      MI.getOperand(LoopRegIdx).setReg(LoopReg);
      MI.removeOperand(InitRegIdx + 1);
      MI.removeOperand(InitRegIdx + 0);
    }
  }

  DebugLoc DL;
  if (Direction == LPD_Front) {
    Preheader->ReplaceUsesOfBlockWith(Loop, NewBB);
    NewBB->addSuccessor(Loop);
    Loop->replacePhiUsesWith(Preheader, NewBB);
    Preheader->updateTerminator(Loop);
    TII->removeBranch(*NewBB);
    TII->insertBranch(*NewBB, Loop, nullptr, {}, DL);
  } else {
    Loop->replaceSuccessor(Exit, NewBB);
    Exit->replacePhiUsesWith(Loop, NewBB);
    NewBB->addSuccessor(Exit);

    MachineBasicBlock *TBB = nullptr, *FBB = nullptr;
    SmallVector<MachineOperand, 4> Cond;
    bool CanAnalyzeBr = !TII->analyzeBranch(*Loop, TBB, FBB, Cond);
    (void)CanAnalyzeBr;
    assert(CanAnalyzeBr && "Must be able to analyze the loop branch!");
    TII->removeBranch(*Loop);
    TII->insertBranch(*Loop, TBB == Exit ? NewBB : TBB,
                      FBB == Exit ? NewBB : FBB, Cond, DL);
    if (TII->removeBranch(*NewBB) > 0)
      TII->insertBranch(*NewBB, Exit, nullptr, {}, DL);
  }

  return NewBB;
}
