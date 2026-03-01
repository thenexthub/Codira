//===----------------------- R600FrameLowering.cpp ------------------------===//
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

#include "R600FrameLowering.h"
#include "R600Subtarget.h"
#include "vm/core/CodeGen/MachineFrameInfo.h"

using namespace vm::core;

R600FrameLowering::~R600FrameLowering() = default;

/// \returns The number of registers allocated for \p FI.
StackOffset
R600FrameLowering::getFrameIndexReference(const MachineFunction &MF, int FI,
                                          Register &FrameReg) const {
  const MachineFrameInfo &MFI = MF.getFrameInfo();
  const R600RegisterInfo *RI
    = MF.getSubtarget<R600Subtarget>().getRegisterInfo();

  // Fill in FrameReg output argument.
  FrameReg = RI->getFrameRegister(MF);

  // Start the offset at 2 so we don't overwrite work group information.
  // FIXME: We should only do this when the shader actually uses this
  // information.
  unsigned OffsetBytes = 2 * (getStackWidth(MF) * 4);
  int UpperBound = FI == -1 ? MFI.getNumObjects() : FI;

  for (int i = MFI.getObjectIndexBegin(); i < UpperBound; ++i) {
    OffsetBytes = alignTo(OffsetBytes, MFI.getObjectAlign(i));
    OffsetBytes += MFI.getObjectSize(i);
    // Each register holds 4 bytes, so we must always align the offset to at
    // least 4 bytes, so that 2 frame objects won't share the same register.
    OffsetBytes = alignTo(OffsetBytes, Align(4));
  }

  if (FI != -1)
    OffsetBytes = alignTo(OffsetBytes, MFI.getObjectAlign(FI));

  return StackOffset::getFixed(OffsetBytes / (getStackWidth(MF) * 4));
}
