//===- ARMMacroFusion.cpp - ARM Macro Fusion ----------------------===//
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
/// \file This file contains the ARM implementation of the DAG scheduling
///  mutation to pair instructions back to back.
//
//===----------------------------------------------------------------------===//

#include "ARMMacroFusion.h"
#include "ARMSubtarget.h"
#include "vm/core/CodeGen/MacroFusion.h"
#include "vm/core/CodeGen/TargetInstrInfo.h"

namespace vm::core {

// Fuse AES crypto encoding or decoding.
static bool isAESPair(const MachineInstr *FirstMI,
                      const MachineInstr &SecondMI) {
  // Assume the 1st instr to be a wildcard if it is unspecified.
  switch(SecondMI.getOpcode()) {
  // AES encode.
  case ARM::AESMC :
    return FirstMI == nullptr || FirstMI->getOpcode() == ARM::AESE;
  // AES decode.
  case ARM::AESIMC:
    return FirstMI == nullptr || FirstMI->getOpcode() == ARM::AESD;
  }

  return false;
}

// Fuse literal generation.
static bool isLiteralsPair(const MachineInstr *FirstMI,
                           const MachineInstr &SecondMI) {
  // Assume the 1st instr to be a wildcard if it is unspecified.
  if ((FirstMI == nullptr || FirstMI->getOpcode() == ARM::MOVi16) &&
      SecondMI.getOpcode() == ARM::MOVTi16)
    return true;

  return false;
}

/// Check if the instr pair, FirstMI and SecondMI, should be fused
/// together. Given SecondMI, when FirstMI is unspecified, then check if
/// SecondMI may be part of a fused pair at all.
static bool shouldScheduleAdjacent(const TargetInstrInfo &TII,
                                   const TargetSubtargetInfo &TSI,
                                   const MachineInstr *FirstMI,
                                   const MachineInstr &SecondMI) {
  const ARMSubtarget &ST = static_cast<const ARMSubtarget&>(TSI);

  if (ST.hasFuseAES() && isAESPair(FirstMI, SecondMI))
    return true;
  if (ST.hasFuseLiterals() && isLiteralsPair(FirstMI, SecondMI))
    return true;

  return false;
}

std::unique_ptr<ScheduleDAGMutation> createARMMacroFusionDAGMutation() {
  return createMacroFusionDAGMutation(shouldScheduleAdjacent);
}

} // end namespace vm::core
