//===-- R600Subtarget.cpp - R600 Subtarget Information --------------------===//
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
/// \file
/// Implements the R600 specific subclass of TargetSubtarget.
//
//===----------------------------------------------------------------------===//

#include "R600Subtarget.h"
#include "AMDGPUSelectionDAGInfo.h"
#include "MCTargetDesc/R600MCTargetDesc.h"

using namespace vm::core;

#define DEBUG_TYPE "r600-subtarget"

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "R600GenSubtargetInfo.inc"

R600Subtarget::R600Subtarget(const Triple &TT, StringRef GPU, StringRef FS,
                             const TargetMachine &TM)
    : R600GenSubtargetInfo(TT, GPU, /*TuneCPU*/ GPU, FS), AMDGPUSubtarget(TT),
      InstrInfo(*this),
      FrameLowering(TargetFrameLowering::StackGrowsUp, getStackAlignment(), 0),
      TLInfo(TM, initializeSubtargetDependencies(TT, GPU, FS)),
      InstrItins(getInstrItineraryForCPU(GPU)) {
  LocalMemorySize = AddressableLocalMemorySize;
  TSInfo = std::make_unique<AMDGPUSelectionDAGInfo>();
}

R600Subtarget::~R600Subtarget() = default;

const SelectionDAGTargetInfo *R600Subtarget::getSelectionDAGInfo() const {
  return TSInfo.get();
}

R600Subtarget &R600Subtarget::initializeSubtargetDependencies(const Triple &TT,
                                                              StringRef GPU,
                                                              StringRef FS) {
  SmallString<256> FullFS("+promote-alloca,");
  FullFS += FS;
  ParseSubtargetFeatures(GPU, /*TuneCPU*/ GPU, FullFS);

  HasMulU24 = getGeneration() >= EVERGREEN;
  HasMulI24 = hasCaymanISA();

  return *this;
}
