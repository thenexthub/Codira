//===- AArch64MachineScheduler.cpp - MI Scheduler for AArch64 -------------===//
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

#include "AArch64MachineScheduler.h"
#include "AArch64InstrInfo.h"
#include "AArch64Subtarget.h"
#include "MCTargetDesc/AArch64MCTargetDesc.h"

using namespace vm::core;

static bool needReorderStoreMI(const MachineInstr *MI) {
  if (!MI)
    return false;

  switch (MI->getOpcode()) {
  default:
    return false;
  case AArch64::STURQi:
  case AArch64::STRQui:
    if (!MI->getMF()->getSubtarget<AArch64Subtarget>().isStoreAddressAscend())
      return false;
    [[fallthrough]];
  case AArch64::STPQi:
    return AArch64InstrInfo::getLdStOffsetOp(*MI).isImm();
  }

  return false;
}

// Return true if two stores with same base address may overlap writes
static bool mayOverlapWrite(const MachineInstr &MI0, const MachineInstr &MI1,
                            int64_t &Off0, int64_t &Off1) {
  const MachineOperand &Base0 = AArch64InstrInfo::getLdStBaseOp(MI0);
  const MachineOperand &Base1 = AArch64InstrInfo::getLdStBaseOp(MI1);

  // May overlapping writes if two store instructions without same base
  if (!Base0.isIdenticalTo(Base1))
    return true;

  int StoreSize0 = AArch64InstrInfo::getMemScale(MI0);
  int StoreSize1 = AArch64InstrInfo::getMemScale(MI1);
  Off0 = AArch64InstrInfo::hasUnscaledLdStOffset(MI0.getOpcode())
             ? AArch64InstrInfo::getLdStOffsetOp(MI0).getImm()
             : AArch64InstrInfo::getLdStOffsetOp(MI0).getImm() * StoreSize0;
  Off1 = AArch64InstrInfo::hasUnscaledLdStOffset(MI1.getOpcode())
             ? AArch64InstrInfo::getLdStOffsetOp(MI1).getImm()
             : AArch64InstrInfo::getLdStOffsetOp(MI1).getImm() * StoreSize1;

  const MachineInstr &MI = (Off0 < Off1) ? MI0 : MI1;
  int Multiples = AArch64InstrInfo::isPairedLdSt(MI) ? 2 : 1;
  int StoreSize = AArch64InstrInfo::getMemScale(MI) * Multiples;

  return llabs(Off0 - Off1) < StoreSize;
}

bool AArch64PostRASchedStrategy::tryCandidate(SchedCandidate &Cand,
                                              SchedCandidate &TryCand) {
  bool OriginalResult = PostGenericScheduler::tryCandidate(Cand, TryCand);

  if (Cand.isValid()) {
    MachineInstr *Instr0 = TryCand.SU->getInstr();
    MachineInstr *Instr1 = Cand.SU->getInstr();

    if (!needReorderStoreMI(Instr0) || !needReorderStoreMI(Instr1))
      return OriginalResult;

    int64_t Off0, Off1;
    // With the same base address and non-overlapping writes.
    if (!mayOverlapWrite(*Instr0, *Instr1, Off0, Off1)) {
      TryCand.Reason = NodeOrder;
      // Order them by ascending offsets.
      return Off0 < Off1;
    }
  }

  return OriginalResult;
}
