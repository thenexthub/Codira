//===- LiveIntervalCalc.cpp - Calculate live interval --------------------===//
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
// Implementation of the LiveIntervalCalc class.
//
//===----------------------------------------------------------------------===//

#include "vm/core/CodeGen/LiveIntervalCalc.h"
#include "vm/core/ADT/SmallVector.h"
#include "vm/core/CodeGen/LiveInterval.h"
#include "vm/core/CodeGen/MachineInstr.h"
#include "vm/core/CodeGen/MachineOperand.h"
#include "vm/core/CodeGen/MachineRegisterInfo.h"
#include "vm/core/CodeGen/SlotIndexes.h"
#include "vm/core/CodeGen/TargetRegisterInfo.h"
#include "vm/core/MC/LaneBitmask.h"
#include <cassert>

using namespace vm::core;

#define DEBUG_TYPE "regalloc"

// Reserve an address that indicates a value that is known to be "undef".
static VNInfo UndefVNI(0xbad, SlotIndex());

static void createDeadDef(SlotIndexes &Indexes, VNInfo::Allocator &Alloc,
                          LiveRange &LR, const MachineOperand &MO) {
  const MachineInstr &MI = *MO.getParent();
  SlotIndex DefIdx =
      Indexes.getInstructionIndex(MI).getRegSlot(MO.isEarlyClobber());

  // Create the def in LR. This may find an existing def.
  LR.createDeadDef(DefIdx, Alloc);
}

void LiveIntervalCalc::calculate(LiveInterval &LI, bool TrackSubRegs) {
  const MachineRegisterInfo *MRI = getRegInfo();
  SlotIndexes *Indexes = getIndexes();
  VNInfo::Allocator *Alloc = getVNAlloc();

  assert(MRI && Indexes && "call reset() first");

  // Step 1: Create minimal live segments for every definition of Reg.
  // Visit all def operands. If the same instruction has multiple defs of Reg,
  // createDeadDef() will deduplicate.
  const TargetRegisterInfo &TRI = *MRI->getTargetRegisterInfo();
  Register Reg = LI.reg();
  for (const MachineOperand &MO : MRI->reg_nodbg_operands(Reg)) {
    if (!MO.isDef() && !MO.readsReg())
      continue;

    unsigned SubReg = MO.getSubReg();
    if (LI.hasSubRanges() || (SubReg != 0 && TrackSubRegs)) {
      LaneBitmask SubMask = SubReg != 0 ? TRI.getSubRegIndexLaneMask(SubReg)
                                        : MRI->getMaxLaneMaskForVReg(Reg);
      // If this is the first time we see a subregister def, initialize
      // subranges by creating a copy of the main range.
      if (!LI.hasSubRanges() && !LI.empty()) {
        LaneBitmask ClassMask = MRI->getMaxLaneMaskForVReg(Reg);
        LI.createSubRangeFrom(*Alloc, ClassMask, LI);
      }

      LI.refineSubRanges(
          *Alloc, SubMask,
          [&MO, Indexes, Alloc](LiveInterval::SubRange &SR) {
            if (MO.isDef())
              createDeadDef(*Indexes, *Alloc, SR, MO);
          },
          *Indexes, TRI);
    }

    // Create the def in the main liverange. We do not have to do this if
    // subranges are tracked as we recreate the main range later in this case.
    if (MO.isDef() && !LI.hasSubRanges())
      createDeadDef(*Indexes, *Alloc, LI, MO);
  }

  // We may have created empty live ranges for partially undefined uses, we
  // can't keep them because we won't find defs in them later.
  LI.removeEmptySubRanges();

  const MachineFunction *MF = getMachineFunction();
  MachineDominatorTree *DomTree = getDomTree();
  // Step 2: Extend live segments to all uses, constructing SSA form as
  // necessary.
  if (LI.hasSubRanges()) {
    for (LiveInterval::SubRange &S : LI.subranges()) {
      LiveIntervalCalc SubLIC;
      SubLIC.reset(MF, Indexes, DomTree, Alloc);
      SubLIC.extendToUses(S, Reg, S.LaneMask, &LI);
    }
    LI.clear();
    constructMainRangeFromSubranges(LI);
  } else {
    resetLiveOutMap();
    extendToUses(LI, Reg, LaneBitmask::getAll());
  }
}

void LiveIntervalCalc::constructMainRangeFromSubranges(LiveInterval &LI) {
  // First create dead defs at all defs found in subranges.
  LiveRange &MainRange = LI;
  assert(MainRange.segments.empty() && MainRange.valnos.empty() &&
         "Expect empty main liverange");

  VNInfo::Allocator *Alloc = getVNAlloc();
  for (const LiveInterval::SubRange &SR : LI.subranges()) {
    for (const VNInfo *VNI : SR.valnos) {
      if (!VNI->isUnused() && !VNI->isPHIDef())
        MainRange.createDeadDef(VNI->def, *Alloc);
    }
  }
  resetLiveOutMap();
  extendToUses(MainRange, LI.reg(), LaneBitmask::getAll(), &LI);
}

void LiveIntervalCalc::createDeadDefs(LiveRange &LR, Register Reg) {
  const MachineRegisterInfo *MRI = getRegInfo();
  SlotIndexes *Indexes = getIndexes();
  VNInfo::Allocator *Alloc = getVNAlloc();
  assert(MRI && Indexes && "call reset() first");

  // Visit all def operands. If the same instruction has multiple defs of Reg,
  // LR.createDeadDef() will deduplicate.
  for (MachineOperand &MO : MRI->def_operands(Reg))
    createDeadDef(*Indexes, *Alloc, LR, MO);
}

void LiveIntervalCalc::extendToUses(LiveRange &LR, Register Reg,
                                    LaneBitmask Mask, LiveInterval *LI) {
  const MachineRegisterInfo *MRI = getRegInfo();
  SlotIndexes *Indexes = getIndexes();
  SmallVector<SlotIndex, 4> Undefs;
  if (LI != nullptr)
    LI->computeSubRangeUndefs(Undefs, Mask, *MRI, *Indexes);

  // Visit all operands that read Reg. This may include partial defs.
  bool IsSubRange = !Mask.all();
  const TargetRegisterInfo &TRI = *MRI->getTargetRegisterInfo();
  for (MachineOperand &MO : MRI->reg_nodbg_operands(Reg)) {
    // Clear all kill flags. They will be reinserted after register allocation
    // by LiveIntervals::addKillFlags().
    if (MO.isUse())
      MO.setIsKill(false);
    // MO::readsReg returns "true" for subregister defs. This is for keeping
    // liveness of the entire register (i.e. for the main range of the live
    // interval). For subranges, definitions of non-overlapping subregisters
    // do not count as uses.
    if (!MO.readsReg() || (IsSubRange && MO.isDef()))
      continue;

    unsigned SubReg = MO.getSubReg();
    if (SubReg != 0) {
      LaneBitmask SLM = TRI.getSubRegIndexLaneMask(SubReg);
      if (MO.isDef())
        SLM = ~SLM;
      // Ignore uses not reading the current (sub)range.
      if ((SLM & Mask).none())
        continue;
    }

    // Determine the actual place of the use.
    const MachineInstr *MI = MO.getParent();
    unsigned OpNo = (&MO - &MI->getOperand(0));
    SlotIndex UseIdx;
    if (MI->isPHI()) {
      assert(!MO.isDef() && "Cannot handle PHI def of partial register.");
      // The actual place where a phi operand is used is the end of the pred
      // MBB. PHI operands are paired: (Reg, PredMBB).
      UseIdx = Indexes->getMBBEndIdx(MI->getOperand(OpNo + 1).getMBB());
    } else {
      // Check for early-clobber redefs.
      bool isEarlyClobber = false;
      unsigned DefIdx;
      if (MO.isDef())
        isEarlyClobber = MO.isEarlyClobber();
      else if (MI->isRegTiedToDefOperand(OpNo, &DefIdx)) {
        // FIXME: This would be a lot easier if tied early-clobber uses also
        // had an early-clobber flag.
        isEarlyClobber = MI->getOperand(DefIdx).isEarlyClobber();
      }
      UseIdx = Indexes->getInstructionIndex(*MI).getRegSlot(isEarlyClobber);
    }

    // MI is reading Reg. We may have visited MI before if it happens to be
    // reading Reg multiple times. That is OK, extend() is idempotent.
    extend(LR, UseIdx, Reg, Undefs);
  }
}
