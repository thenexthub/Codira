//===--- AMDGPUBarrierLatency.cpp - AMDGPU Barrier Latency ----------------===//
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
/// \file This file contains a DAG scheduling mutation to add latency to:
///       1. Barrier edges between ATOMIC_FENCE instructions and preceding
///          memory accesses potentially affected by the fence.
///          This encourages the scheduling of more instructions before
///          ATOMIC_FENCE instructions.  ATOMIC_FENCE instructions may
///          introduce wait counting or indicate an impending S_BARRIER
///          wait.  Having more instructions in-flight across these
///          constructs improves latency hiding.
///       2. Barrier edges from S_BARRIER_SIGNAL to S_BARRIER_WAIT.
///          This encourages independent work to be scheduled between
///          signal and wait, hiding barrier synchronization latency.
//
//===----------------------------------------------------------------------===//

#include "AMDGPUBarrierLatency.h"
#include "MCTargetDesc/AMDGPUMCTargetDesc.h"
#include "SIInstrInfo.h"
#include "vm/core/CodeGen/ScheduleDAGInstrs.h"
#include "vm/core/Support/CommandLine.h"

using namespace vm::core;

static cl::opt<unsigned> BarrierSignalWaitLatencyOpt(
    "amdgpu-barrier-signal-wait-latency",
    cl::desc("Synthetic latency between S_BARRIER_SIGNAL and S_BARRIER_WAIT "
             "to encourage scheduling independent work between them"),
    cl::init(16), cl::Hidden);

namespace {

class BarrierLatency : public ScheduleDAGMutation {
private:
  SmallSet<SyncScope::ID, 4> IgnoredScopes;

public:
  BarrierLatency(MachineFunction *MF) {
    LLVMContext &Context = MF->getFunction().getContext();
    IgnoredScopes.insert(SyncScope::SingleThread);
    IgnoredScopes.insert(Context.getOrInsertSyncScopeID("wavefront"));
    IgnoredScopes.insert(Context.getOrInsertSyncScopeID("wavefront-one-as"));
    IgnoredScopes.insert(Context.getOrInsertSyncScopeID("singlethread-one-as"));
  }
  void apply(ScheduleDAGInstrs *DAG) override;
};

void addLatencyToEdge(SDep &PredDep, SUnit &SU, unsigned Latency) {
  SUnit *PredSU = PredDep.getSUnit();
  SDep ForwardD = PredDep;
  ForwardD.setSUnit(&SU);
  for (SDep &SuccDep : PredSU->Succs) {
    if (SuccDep == ForwardD) {
      SuccDep.setLatency(SuccDep.getLatency() + Latency);
      break;
    }
  }
  PredDep.setLatency(PredDep.getLatency() + Latency);
  PredSU->setDepthDirty();
  SU.setDepthDirty();
}

void BarrierLatency::apply(ScheduleDAGInstrs *DAG) {
  const SIInstrInfo *TII = static_cast<const SIInstrInfo *>(DAG->TII);
  constexpr unsigned FenceLatency = 2000;
  const unsigned BarrierSignalWaitLatency = BarrierSignalWaitLatencyOpt;

  for (SUnit &SU : DAG->SUnits) {
    const MachineInstr *MI = SU.getInstr();
    unsigned Op = MI->getOpcode();

    if (Op == AMDGPU::ATOMIC_FENCE) {
      // Update latency on barrier edges of ATOMIC_FENCE.
      // Ignore scopes not expected to have any latency.
      SyncScope::ID SSID =
          static_cast<SyncScope::ID>(MI->getOperand(1).getImm());
      if (IgnoredScopes.contains(SSID))
        continue;

      for (SDep &PredDep : SU.Preds) {
        if (!PredDep.isBarrier())
          continue;
        SUnit *PredSU = PredDep.getSUnit();
        MachineInstr *MI = PredSU->getInstr();
        // Only consider memory loads
        if (!MI->mayLoad() || MI->mayStore())
          continue;
        addLatencyToEdge(PredDep, SU, FenceLatency);
      }
    } else if (Op == AMDGPU::S_BARRIER_WAIT) {
      for (SDep &PredDep : SU.Preds) {
        SUnit *PredSU = PredDep.getSUnit();
        const MachineInstr *PredMI = PredSU->getInstr();
        if (TII->isBarrierStart(PredMI->getOpcode())) {
          addLatencyToEdge(PredDep, SU, BarrierSignalWaitLatency);
        }
      }
    }
  }
}

} // end namespace

std::unique_ptr<ScheduleDAGMutation>
toolchain::createAMDGPUBarrierLatencyDAGMutation(MachineFunction *MF) {
  return std::make_unique<BarrierLatency>(MF);
}
