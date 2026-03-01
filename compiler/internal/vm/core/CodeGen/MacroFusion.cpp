//===- MacroFusion.cpp - Macro Fusion -------------------------------------===//
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
/// \file This file contains the implementation of the DAG scheduling mutation
/// to pair instructions back to back.
//
//===----------------------------------------------------------------------===//

#include "vm/core/CodeGen/MacroFusion.h"
#include "vm/core/ADT/Statistic.h"
#include "vm/core/CodeGen/MachineInstr.h"
#include "vm/core/CodeGen/ScheduleDAG.h"
#include "vm/core/CodeGen/ScheduleDAGInstrs.h"
#include "vm/core/CodeGen/ScheduleDAGMutation.h"
#include "vm/core/CodeGen/TargetInstrInfo.h"
#include "vm/core/Support/CommandLine.h"
#include "vm/core/Support/Debug.h"
#include "vm/core/Support/raw_ostream.h"

#define DEBUG_TYPE "machine-scheduler"

STATISTIC(NumFused, "Number of instr pairs fused");

using namespace vm::core;

static cl::opt<bool> EnableMacroFusion("misched-fusion", cl::Hidden,
  cl::desc("Enable scheduling for macro fusion."), cl::init(true));

static bool isHazard(const SDep &Dep) {
  return Dep.getKind() == SDep::Anti || Dep.getKind() == SDep::Output;
}

static SUnit *getPredClusterSU(const SUnit &SU) {
  for (const SDep &SI : SU.Preds)
    if (SI.isCluster())
      return SI.getSUnit();

  return nullptr;
}

bool toolchain::hasLessThanNumFused(const SUnit &SU, unsigned FuseLimit) {
  unsigned Num = 1;
  const SUnit *CurrentSU = &SU;
  while ((CurrentSU = getPredClusterSU(*CurrentSU)) && Num < FuseLimit) Num ++;
  return Num < FuseLimit;
}

bool toolchain::fuseInstructionPair(ScheduleDAGInstrs &DAG, SUnit &FirstSU,
                               SUnit &SecondSU) {
  // Check that neither instr is already paired with another along the edge
  // between them.
  for (SDep &SI : FirstSU.Succs)
    if (SI.isCluster())
      return false;

  for (SDep &SI : SecondSU.Preds)
    if (SI.isCluster())
      return false;

  assert(FirstSU.ParentClusterIdx == InvalidClusterId &&
         SecondSU.ParentClusterIdx == InvalidClusterId);

  // Though the reachability checks above could be made more generic,
  // perhaps as part of ScheduleDAGInstrs::addEdge(), since such edges are valid,
  // the extra computation cost makes it less interesting in general cases.

  // Create a single weak edge between the adjacent instrs. The only effect is
  // to cause bottom-up scheduling to heavily prioritize the clustered instrs.
  if (!DAG.addEdge(&SecondSU, SDep(&FirstSU, SDep::Cluster)))
    return false;

  auto &Clusters = DAG.getClusters();

  FirstSU.ParentClusterIdx = Clusters.size();
  SecondSU.ParentClusterIdx = Clusters.size();

  SmallPtrSet<SUnit *, 8> Cluster{{&FirstSU, &SecondSU}};
  Clusters.push_back(Cluster);

  // TODO - If we want to chain more than two instructions, we need to create
  // artifical edges to make dependencies from the FirstSU also dependent
  // on other chained instructions, and other chained instructions also
  // dependent on the dependencies of the SecondSU, to prevent them from being
  // scheduled into these chained instructions.
  assert(hasLessThanNumFused(FirstSU, 2) &&
         "Currently we only support chaining together two instructions");

  // Adjust the latency between both instrs.
  for (SDep &SI : FirstSU.Succs)
    if (SI.getSUnit() == &SecondSU)
      SI.setLatency(0);

  for (SDep &SI : SecondSU.Preds)
    if (SI.getSUnit() == &FirstSU)
      SI.setLatency(0);

  LLVM_DEBUG(
      dbgs() << "Macro fuse: "; DAG.dumpNodeName(FirstSU); dbgs() << " - ";
      DAG.dumpNodeName(SecondSU); dbgs() << " /  ";
      dbgs() << DAG.TII->getName(FirstSU.getInstr()->getOpcode()) << " - "
             << DAG.TII->getName(SecondSU.getInstr()->getOpcode()) << '\n';);

  // Make data dependencies from the FirstSU also dependent on the SecondSU to
  // prevent them from being scheduled between the FirstSU and the SecondSU.
  if (&SecondSU != &DAG.ExitSU)
    for (const SDep &SI : FirstSU.Succs) {
      SUnit *SU = SI.getSUnit();
      if (SI.isWeak() || isHazard(SI) ||
          SU == &DAG.ExitSU || SU == &SecondSU || SU->isPred(&SecondSU))
        continue;
      LLVM_DEBUG(dbgs() << "  Bind "; DAG.dumpNodeName(SecondSU);
                 dbgs() << " - "; DAG.dumpNodeName(*SU); dbgs() << '\n';);
      DAG.addEdge(SU, SDep(&SecondSU, SDep::Artificial));
    }

  // Make the FirstSU also dependent on the dependencies of the SecondSU to
  // prevent them from being scheduled between the FirstSU and the SecondSU.
  if (&FirstSU != &DAG.EntrySU) {
    for (const SDep &SI : SecondSU.Preds) {
      SUnit *SU = SI.getSUnit();
      if (SI.isWeak() || isHazard(SI) || &FirstSU == SU || FirstSU.isSucc(SU))
        continue;
      LLVM_DEBUG(dbgs() << "  Bind "; DAG.dumpNodeName(*SU); dbgs() << " - ";
                 DAG.dumpNodeName(FirstSU); dbgs() << '\n';);
      DAG.addEdge(&FirstSU, SDep(SU, SDep::Artificial));
    }
    // ExitSU comes last by design, which acts like an implicit dependency
    // between ExitSU and any bottom root in the graph. We should transfer
    // this to FirstSU as well.
    if (&SecondSU == &DAG.ExitSU) {
      for (SUnit &SU : DAG.SUnits) {
        if (SU.Succs.empty())
          DAG.addEdge(&FirstSU, SDep(&SU, SDep::Artificial));
      }
    }
  }

  ++NumFused;
  return true;
}

namespace {

/// Post-process the DAG to create cluster edges between instrs that may
/// be fused by the processor into a single operation.
class MacroFusion : public ScheduleDAGMutation {
  std::vector<MacroFusionPredTy> Predicates;
  bool FuseBlock;
  bool scheduleAdjacentImpl(ScheduleDAGInstrs &DAG, SUnit &AnchorSU);

public:
  MacroFusion(ArrayRef<MacroFusionPredTy> Predicates, bool FuseBlock)
      : Predicates(Predicates.begin(), Predicates.end()), FuseBlock(FuseBlock) {
  }

  void apply(ScheduleDAGInstrs *DAGInstrs) override;

  bool shouldScheduleAdjacent(const TargetInstrInfo &TII,
                              const TargetSubtargetInfo &STI,
                              const MachineInstr *FirstMI,
                              const MachineInstr &SecondMI);
};

} // end anonymous namespace

bool MacroFusion::shouldScheduleAdjacent(const TargetInstrInfo &TII,
                                         const TargetSubtargetInfo &STI,
                                         const MachineInstr *FirstMI,
                                         const MachineInstr &SecondMI) {
  return toolchain::any_of(Predicates, [&](MacroFusionPredTy Predicate) {
    return Predicate(TII, STI, FirstMI, SecondMI);
  });
}

void MacroFusion::apply(ScheduleDAGInstrs *DAG) {
  if (FuseBlock)
    // For each of the SUnits in the scheduling block, try to fuse the instr in
    // it with one in its predecessors.
    for (SUnit &ISU : DAG->SUnits)
        scheduleAdjacentImpl(*DAG, ISU);

  if (DAG->ExitSU.getInstr())
    // Try to fuse the instr in the ExitSU with one in its predecessors.
    scheduleAdjacentImpl(*DAG, DAG->ExitSU);
}

/// Implement the fusion of instr pairs in the scheduling DAG,
/// anchored at the instr in AnchorSU..
bool MacroFusion::scheduleAdjacentImpl(ScheduleDAGInstrs &DAG, SUnit &AnchorSU) {
  const MachineInstr &AnchorMI = *AnchorSU.getInstr();
  const TargetInstrInfo &TII = *DAG.TII;
  const TargetSubtargetInfo &ST = DAG.MF.getSubtarget();

  // Check if the anchor instr may be fused.
  if (!shouldScheduleAdjacent(TII, ST, nullptr, AnchorMI))
    return false;

  // Explorer for fusion candidates among the dependencies of the anchor instr.
  for (SDep &Dep : AnchorSU.Preds) {
    // Ignore dependencies other than data or strong ordering.
    if (Dep.isWeak() || isHazard(Dep))
      continue;

    SUnit &DepSU = *Dep.getSUnit();
    if (DepSU.isBoundaryNode())
      continue;

    // Only chain two instructions together at most.
    const MachineInstr *DepMI = DepSU.getInstr();
    if (!hasLessThanNumFused(DepSU, 2) ||
        !shouldScheduleAdjacent(TII, ST, DepMI, AnchorMI))
      continue;

    if (fuseInstructionPair(DAG, DepSU, AnchorSU))
      return true;
  }

  return false;
}

std::unique_ptr<ScheduleDAGMutation>
toolchain::createMacroFusionDAGMutation(ArrayRef<MacroFusionPredTy> Predicates,
                                   bool BranchOnly) {
  if (EnableMacroFusion)
    return std::make_unique<MacroFusion>(Predicates, !BranchOnly);
  return nullptr;
}
