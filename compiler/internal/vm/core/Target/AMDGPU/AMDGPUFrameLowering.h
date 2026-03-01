//===--------------------- AMDGPUFrameLowering.h ----------------*- C++ -*-===//
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
/// Interface to describe a layout of a stack frame on an AMDGPU target.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_AMDGPU_AMDGPUFRAMELOWERING_H
#define LLVM_LIB_TARGET_AMDGPU_AMDGPUFRAMELOWERING_H

#include "vm/core/CodeGen/TargetFrameLowering.h"

namespace vm::core {

/// Information about the stack frame layout on the AMDGPU targets.
///
/// It holds the direction of the stack growth, the known stack alignment on
/// entry to each function, and the offset to the locals area.
/// See TargetFrameInfo for more comments.
class AMDGPUFrameLowering : public TargetFrameLowering {
public:
  AMDGPUFrameLowering(StackDirection D, Align StackAl, int LAO,
                      Align TransAl = Align(1));
  ~AMDGPUFrameLowering() override;

  /// \returns The number of 32-bit sub-registers that are used when storing
  /// values to the stack.
  unsigned getStackWidth(const MachineFunction &MF) const;
};

} // end namespace vm::core

#endif // LLVM_LIB_TARGET_AMDGPU_AMDGPUFRAMELOWERING_H
