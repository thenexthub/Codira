//===-- VESubtarget.cpp - VE Subtarget Information ------------------------===//
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
// This file implements the VE specific subclass of TargetSubtargetInfo.
//
//===----------------------------------------------------------------------===//

#include "VESubtarget.h"
#include "VESelectionDAGInfo.h"
#include "vm/core/MC/TargetRegistry.h"

using namespace vm::core;

#define DEBUG_TYPE "ve-subtarget"

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "VEGenSubtargetInfo.inc"

void VESubtarget::anchor() {}

VESubtarget &VESubtarget::initializeSubtargetDependencies(StringRef CPU,
                                                          StringRef FS) {
  // Default feature settings
  EnableVPU = false;

  // Determine default and user specified characteristics
  std::string CPUName = std::string(CPU);
  if (CPUName.empty())
    CPUName = "generic";

  // Parse features string.
  ParseSubtargetFeatures(CPUName, /*TuneCPU=*/CPU, FS);

  return *this;
}

VESubtarget::VESubtarget(const Triple &TT, const std::string &CPU,
                         const std::string &FS, const TargetMachine &TM)
    : VEGenSubtargetInfo(TT, CPU, /*TuneCPU=*/CPU, FS), TargetTriple(TT),
      InstrInfo(initializeSubtargetDependencies(CPU, FS)), TLInfo(TM, *this),
      FrameLowering(*this) {
  TSInfo = std::make_unique<VESelectionDAGInfo>();
}

VESubtarget::~VESubtarget() = default;

const SelectionDAGTargetInfo *VESubtarget::getSelectionDAGInfo() const {
  return TSInfo.get();
}

uint64_t VESubtarget::getAdjustedFrameSize(uint64_t FrameSize) const {
  // Calculate adjusted frame size by adding the size of RSA frame,
  // return address, and frame poitner as described in VEFrameLowering.cpp.
  const VEFrameLowering *TFL = getFrameLowering();

  FrameSize += getRsaSize();
  FrameSize = alignTo(FrameSize, TFL->getStackAlign());

  return FrameSize;
}

bool VESubtarget::enableMachineScheduler() const { return true; }
