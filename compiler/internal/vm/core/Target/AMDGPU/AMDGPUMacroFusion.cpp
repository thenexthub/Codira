//===--- AMDGPUMacroFusion.cpp - AMDGPU Macro Fusion ----------------------===//
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
/// \file This file contains the AMDGPU implementation of the DAG scheduling
///  mutation to pair instructions back to back.
//
//===----------------------------------------------------------------------===//

#include "AMDGPUMacroFusion.h"
#include "MCTargetDesc/AMDGPUMCTargetDesc.h"
#include "SIInstrInfo.h"
#include "vm/core/CodeGen/MacroFusion.h"

using namespace vm::core;

namespace {

/// Check if the instr pair, FirstMI and SecondMI, should be fused
/// together. Given SecondMI, when FirstMI is unspecified, then check if
/// SecondMI may be part of a fused pair at all.
static bool shouldScheduleAdjacent(const TargetInstrInfo &TII_,
                                   const TargetSubtargetInfo &TSI,
                                   const MachineInstr *FirstMI,
                                   const MachineInstr &SecondMI) {
  const SIInstrInfo &TII = static_cast<const SIInstrInfo&>(TII_);

  switch (SecondMI.getOpcode()) {
  case AMDGPU::V_ADDC_U32_e64:
  case AMDGPU::V_SUBB_U32_e64:
  case AMDGPU::V_SUBBREV_U32_e64:
  case AMDGPU::V_CNDMASK_B32_e64: {
    // Try to cluster defs of condition registers to their uses. This improves
    // the chance VCC will be available which will allow shrinking to VOP2
    // encodings.
    if (!FirstMI)
      return true;

    const MachineBasicBlock &MBB = *FirstMI->getParent();
    const MachineRegisterInfo &MRI = MBB.getParent()->getRegInfo();
    const TargetRegisterInfo *TRI = MRI.getTargetRegisterInfo();
    const MachineOperand *Src2 = TII.getNamedOperand(SecondMI,
                                                     AMDGPU::OpName::src2);
    return FirstMI->definesRegister(Src2->getReg(), TRI);
  }
  default:
    return false;
  }

  return false;
}

} // end namespace


namespace vm::core {

std::unique_ptr<ScheduleDAGMutation> createAMDGPUMacroFusionDAGMutation() {
  return createMacroFusionDAGMutation(shouldScheduleAdjacent);
}

} // end namespace vm::core
