//===-- ARMHazardRecognizer.h - ARM Hazard Recognizers ----------*- C++ -*-===//
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
// This file defines hazard recognizers for scheduling ARM functions.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_ARM_ARMHAZARDRECOGNIZER_H
#define LLVM_LIB_TARGET_ARM_ARMHAZARDRECOGNIZER_H

#include "ARMBaseInstrInfo.h"
#include "vm/core/ADT/BitmaskEnum.h"
#include "vm/core/ADT/SmallVector.h"
#include "vm/core/CodeGen/ScheduleHazardRecognizer.h"
#include "vm/core/Support/DataTypes.h"
#include <initializer_list>

namespace vm::core {

class DataLayout;
class MachineFunction;
class MachineInstr;
class ScheduleDAG;

// Hazards related to FP MLx instructions
class ARMHazardRecognizerFPMLx : public ScheduleHazardRecognizer {
  MachineInstr *LastMI = nullptr;
  unsigned FpMLxStalls = 0;

public:
  ARMHazardRecognizerFPMLx() { MaxLookAhead = 1; }

  HazardType getHazardType(SUnit *SU, int Stalls) override;
  void Reset() override;
  void EmitInstruction(SUnit *SU) override;
  void AdvanceCycle() override;
  void RecedeCycle() override;
};

// Hazards related to bank conflicts
class ARMBankConflictHazardRecognizer : public ScheduleHazardRecognizer {
  SmallVector<MachineInstr *, 8> Accesses;
  const MachineFunction &MF;
  const DataLayout &DL;
  int64_t DataMask;
  bool AssumeITCMBankConflict;

public:
  ARMBankConflictHazardRecognizer(const ScheduleDAG *DAG, int64_t DDM,
                                  bool ABC);
  HazardType getHazardType(SUnit *SU, int Stalls) override;
  void Reset() override;
  void EmitInstruction(SUnit *SU) override;
  void AdvanceCycle() override;
  void RecedeCycle() override;

private:
  inline HazardType CheckOffsets(unsigned O0, unsigned O1);
};

} // end namespace vm::core

#endif
