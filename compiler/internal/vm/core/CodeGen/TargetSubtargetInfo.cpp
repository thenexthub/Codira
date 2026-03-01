//===- TargetSubtargetInfo.cpp - General Target Information ----------------==//
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
/// \file This file describes the general parts of a Subtarget.
//
//===----------------------------------------------------------------------===//

#include "vm/core/CodeGen/TargetSubtargetInfo.h"

using namespace vm::core;

TargetSubtargetInfo::TargetSubtargetInfo(
    const Triple &TT, StringRef CPU, StringRef TuneCPU, StringRef FS,
    ArrayRef<StringRef> PN, ArrayRef<SubtargetFeatureKV> PF,
    ArrayRef<SubtargetSubTypeKV> PD, const MCWriteProcResEntry *WPR,
    const MCWriteLatencyEntry *WL, const MCReadAdvanceEntry *RA,
    const InstrStage *IS, const unsigned *OC, const unsigned *FP)
    : MCSubtargetInfo(TT, CPU, TuneCPU, FS, PN, PF, PD, WPR, WL, RA, IS, OC,
                      FP) {}

TargetSubtargetInfo::~TargetSubtargetInfo() = default;

bool TargetSubtargetInfo::enableAtomicExpand() const {
  return true;
}

bool TargetSubtargetInfo::enableIndirectBrExpand() const {
  return false;
}

bool TargetSubtargetInfo::enableMachineScheduler() const {
  return false;
}

bool TargetSubtargetInfo::enableJoinGlobalCopies() const {
  return enableMachineScheduler();
}

bool TargetSubtargetInfo::enableRALocalReassignment(
    CodeGenOptLevel OptLevel) const {
  return true;
}

bool TargetSubtargetInfo::enablePostRAScheduler() const {
  return getSchedModel().PostRAScheduler;
}

bool TargetSubtargetInfo::enablePostRAMachineScheduler() const {
  return enableMachineScheduler() && enablePostRAScheduler();
}

bool TargetSubtargetInfo::useAA() const {
  return false;
}

void TargetSubtargetInfo::mirFileLoaded(MachineFunction &MF) const { }
