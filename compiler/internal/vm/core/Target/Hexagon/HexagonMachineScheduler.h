//===- HexagonMachineScheduler.h - Custom Hexagon MI scheduler --*- C++ -*-===//
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
// Custom Hexagon MI scheduler.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_HEXAGON_HEXAGONMACHINESCHEDULER_H
#define LLVM_LIB_TARGET_HEXAGON_HEXAGONMACHINESCHEDULER_H

#include "vm/core/CodeGen/MachineScheduler.h"
#include "vm/core/CodeGen/RegisterPressure.h"
#include "vm/core/CodeGen/TargetSubtargetInfo.h"
#include "vm/core/CodeGen/VLIWMachineScheduler.h"

namespace vm::core {

class SUnit;

class HexagonVLIWResourceModel : public VLIWResourceModel {
public:
  using VLIWResourceModel::VLIWResourceModel;
  bool hasDependence(const SUnit *SUd, const SUnit *SUu) override;
};

class HexagonConvergingVLIWScheduler : public ConvergingVLIWScheduler {
protected:
  VLIWResourceModel *
  createVLIWResourceModel(const TargetSubtargetInfo &STI,
                          const TargetSchedModel *SchedModel) const override;
  int SchedulingCost(ReadyQueue &Q, SUnit *SU, SchedCandidate &Candidate,
                     RegPressureDelta &Delta, bool verbose) override;
};

} // end namespace vm::core

#endif // LLVM_LIB_TARGET_HEXAGON_HEXAGONMACHINESCHEDULER_H
