//===- HexagonMachineScheduler.cpp - MI Scheduler for Hexagon -------------===//
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
// MachineScheduler schedules machine instructions after phi elimination. It
// preserves LiveIntervals so it can be invoked before register allocation.
//
//===----------------------------------------------------------------------===//

#include "HexagonMachineScheduler.h"
#include "HexagonInstrInfo.h"
#include "HexagonSubtarget.h"
#include "vm/core/CodeGen/MachineScheduler.h"
#include "vm/core/CodeGen/ScheduleDAG.h"
#include "vm/core/CodeGen/VLIWMachineScheduler.h"

using namespace vm::core;

#define DEBUG_TYPE "machine-scheduler"

/// Return true if there is a dependence between SUd and SUu.
bool HexagonVLIWResourceModel::hasDependence(const SUnit *SUd,
                                             const SUnit *SUu) {
  const auto *QII = static_cast<const HexagonInstrInfo *>(TII);

  // Enable .cur formation.
  if (QII->mayBeCurLoad(*SUd->getInstr()))
    return false;

  if (QII->canExecuteInBundle(*SUd->getInstr(), *SUu->getInstr()))
    return false;

  return VLIWResourceModel::hasDependence(SUd, SUu);
}

VLIWResourceModel *HexagonConvergingVLIWScheduler::createVLIWResourceModel(
    const TargetSubtargetInfo &STI, const TargetSchedModel *SchedModel) const {
  return new HexagonVLIWResourceModel(STI, SchedModel);
}

int HexagonConvergingVLIWScheduler::SchedulingCost(ReadyQueue &Q, SUnit *SU,
                                                   SchedCandidate &Candidate,
                                                   RegPressureDelta &Delta,
                                                   bool verbose) {
  int ResCount =
      ConvergingVLIWScheduler::SchedulingCost(Q, SU, Candidate, Delta, verbose);

  if (!SU || SU->isScheduled)
    return ResCount;

  auto &QST = DAG->MF.getSubtarget<HexagonSubtarget>();
  auto &QII = *QST.getInstrInfo();
  if (SU->isInstr() && QII.mayBeCurLoad(*SU->getInstr())) {
    if (Q.getID() == TopQID &&
        Top.ResourceModel->isResourceAvailable(SU, true)) {
      ResCount += PriorityTwo;
      LLVM_DEBUG(if (verbose) dbgs() << "C|");
    } else if (Q.getID() == BotQID &&
               Bot.ResourceModel->isResourceAvailable(SU, false)) {
      ResCount += PriorityTwo;
      LLVM_DEBUG(if (verbose) dbgs() << "C|");
    }
  }

  return ResCount;
}
