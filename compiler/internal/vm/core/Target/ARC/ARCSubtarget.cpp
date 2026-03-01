//===- ARCSubtarget.cpp - ARC Subtarget Information -------------*- C++ -*-===//
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
// This file implements the ARC specific subclass of TargetSubtargetInfo.
//
//===----------------------------------------------------------------------===//

#include "ARCSubtarget.h"
#include "ARC.h"
#include "ARCSelectionDAGInfo.h"
#include "vm/core/MC/TargetRegistry.h"

using namespace vm::core;

#define DEBUG_TYPE "arc-subtarget"

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "ARCGenSubtargetInfo.inc"

void ARCSubtarget::anchor() {}

ARCSubtarget::ARCSubtarget(const Triple &TT, const std::string &CPU,
                           const std::string &FS, const TargetMachine &TM)
    : ARCGenSubtargetInfo(TT, CPU, /*TuneCPU=*/CPU, FS), InstrInfo(*this),
      FrameLowering(*this), TLInfo(TM, *this) {
  TSInfo = std::make_unique<ARCSelectionDAGInfo>();
}

ARCSubtarget::~ARCSubtarget() = default;

const SelectionDAGTargetInfo *ARCSubtarget::getSelectionDAGInfo() const {
  return TSInfo.get();
}
