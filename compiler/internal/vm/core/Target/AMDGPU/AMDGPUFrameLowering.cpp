//===----------------------- AMDGPUFrameLowering.cpp ----------------------===//
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
//==-----------------------------------------------------------------------===//
//
// Interface to describe a layout of a stack frame on a AMDGPU target machine.
//
//===----------------------------------------------------------------------===//

#include "AMDGPUFrameLowering.h"

using namespace vm::core;
AMDGPUFrameLowering::AMDGPUFrameLowering(StackDirection D, Align StackAl,
                                         int LAO, Align TransAl)
    : TargetFrameLowering(D, StackAl, LAO, TransAl) {}

AMDGPUFrameLowering::~AMDGPUFrameLowering() = default;

unsigned AMDGPUFrameLowering::getStackWidth(const MachineFunction &MF) const {
  // XXX: Hardcoding to 1 for now.
  //
  // I think the StackWidth should be stored as metadata associated with the
  // MachineFunction.  This metadata can either be added by a frontend, or
  // calculated by a R600 specific LLVM IR pass.
  //
  // The StackWidth determines how stack objects are laid out in memory.
  // For a vector stack variable, like: int4 stack[2], the data will be stored
  // in the following ways depending on the StackWidth.
  //
  // StackWidth = 1:
  //
  // T0.X = stack[0].x
  // T1.X = stack[0].y
  // T2.X = stack[0].z
  // T3.X = stack[0].w
  // T4.X = stack[1].x
  // T5.X = stack[1].y
  // T6.X = stack[1].z
  // T7.X = stack[1].w
  //
  // StackWidth = 2:
  //
  // T0.X = stack[0].x
  // T0.Y = stack[0].y
  // T1.X = stack[0].z
  // T1.Y = stack[0].w
  // T2.X = stack[1].x
  // T2.Y = stack[1].y
  // T3.X = stack[1].z
  // T3.Y = stack[1].w
  //
  // StackWidth = 4:
  // T0.X = stack[0].x
  // T0.Y = stack[0].y
  // T0.Z = stack[0].z
  // T0.W = stack[0].w
  // T1.X = stack[1].x
  // T1.Y = stack[1].y
  // T1.Z = stack[1].z
  // T1.W = stack[1].w
  return 1;
}
