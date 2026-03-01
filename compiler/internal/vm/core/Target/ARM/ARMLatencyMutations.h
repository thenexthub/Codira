//===- ARMLatencyMutations.h - ARM Latency Mutations ----------------------===//
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
/// \file This file contains the ARM definition DAG scheduling mutations which
/// change inter-instruction latencies
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_ARM_LATENCYMUTATIONS_H
#define LLVM_LIB_TARGET_ARM_LATENCYMUTATIONS_H

#include "vm/core/CodeGen/MachineScheduler.h"
#include "vm/core/CodeGen/ScheduleDAGMutation.h"

namespace vm::core {

class AAResults;
class ARMBaseInstrInfo;

/// Post-process the DAG to create cluster edges between instrs that may
/// be fused by the processor into a single operation.
class ARMOverrideBypasses : public ScheduleDAGMutation {
public:
  ARMOverrideBypasses(const ARMBaseInstrInfo *t, AAResults *a)
      : ScheduleDAGMutation(), TII(t), AA(a) {}

  void apply(ScheduleDAGInstrs *DAGInstrs) override;

private:
  virtual void modifyBypasses(SUnit &) = 0;

protected:
  const ARMBaseInstrInfo *TII;
  AAResults *AA;
  ScheduleDAGInstrs *DAG = nullptr;

  static void setBidirLatencies(SUnit &SrcSU, SDep &SrcDep, unsigned latency);
  static bool zeroOutputDependences(SUnit &ISU, SDep &Dep);
  unsigned makeBundleAssumptions(SUnit &ISU, SDep &Dep);
  bool memoryRAWHazard(SUnit &ISU, SDep &Dep, unsigned latency);
};

/// Note that you have to add:
///   DAG.addMutation(createARMLatencyMutation(ST, AA));
/// to ARMTargetMachine::createMachineScheduler() to have an effect.
std::unique_ptr<ScheduleDAGMutation>
createARMLatencyMutations(const class ARMSubtarget &, AAResults *AA);

} // namespace vm::core

#endif
