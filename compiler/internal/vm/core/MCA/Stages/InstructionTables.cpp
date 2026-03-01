//===--------------------- InstructionTables.cpp ----------------*- C++ -*-===//
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
///
/// This file implements the method InstructionTables::execute().
/// Method execute() prints a theoretical resource pressure distribution based
/// on the information available in the scheduling model, and without running
/// the pipeline.
///
//===----------------------------------------------------------------------===//

#include "vm/core/MCA/Stages/InstructionTables.h"

namespace vm::core {
namespace mca {

Error InstructionTables::execute(InstRef &IR) {
  const InstrDesc &Desc = IR.getInstruction()->getDesc();
  UsedResources.clear();

  // Identify the resources consumed by this instruction.
  for (const std::pair<uint64_t, ResourceUsage> &Resource :
       Desc.Resources) {
    // Skip zero-cycle resources (i.e., unused resources).
    if (!Resource.second.size())
      continue;
    unsigned Cycles = Resource.second.size();
    unsigned Index = std::distance(Masks.begin(), find(Masks, Resource.first));
    const MCProcResourceDesc &ProcResource = *SM.getProcResource(Index);
    unsigned NumUnits = ProcResource.NumUnits;
    if (!ProcResource.SubUnitsIdxBegin) {
      // The number of cycles consumed by each unit.
      for (unsigned I = 0, E = NumUnits; I < E; ++I) {
        ResourceRef ResourceUnit = std::make_pair(Index, 1U << I);
        UsedResources.emplace_back(
            std::make_pair(ResourceUnit, ReleaseAtCycles(Cycles, NumUnits)));
      }
      continue;
    }

    // This is a group. Obtain the set of resources contained in this
    // group. Some of these resources may implement multiple units.
    // Uniformly distribute Cycles across all of the units.
    for (unsigned I1 = 0; I1 < NumUnits; ++I1) {
      unsigned SubUnitIdx = ProcResource.SubUnitsIdxBegin[I1];
      const MCProcResourceDesc &SubUnit = *SM.getProcResource(SubUnitIdx);
      // Compute the number of cycles consumed by each resource unit.
      for (unsigned I2 = 0, E2 = SubUnit.NumUnits; I2 < E2; ++I2) {
        ResourceRef ResourceUnit = std::make_pair(SubUnitIdx, 1U << I2);
        UsedResources.emplace_back(std::make_pair(
            ResourceUnit,
            ReleaseAtCycles(Cycles, NumUnits * SubUnit.NumUnits)));
      }
    }
  }

  // Send a fake instruction issued event to all the views.
  HWInstructionIssuedEvent Event(IR, UsedResources);
  notifyEvent<HWInstructionIssuedEvent>(Event);
  return ErrorSuccess();
}

} // namespace mca
} // namespace vm::core
