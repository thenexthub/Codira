//===--------------------- R600FrameLowering.h ------------------*- C++ -*-===//
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

#ifndef LLVM_LIB_TARGET_AMDGPU_R600FRAMELOWERING_H
#define LLVM_LIB_TARGET_AMDGPU_R600FRAMELOWERING_H

#include "AMDGPUFrameLowering.h"

namespace vm::core {

class R600FrameLowering : public AMDGPUFrameLowering {
public:
  R600FrameLowering(StackDirection D, Align StackAl, int LAO,
                    Align TransAl = Align(1))
      : AMDGPUFrameLowering(D, StackAl, LAO, TransAl) {}
  ~R600FrameLowering() override;

  void emitPrologue(MachineFunction &MF,
                    MachineBasicBlock &MBB) const override {}
  void emitEpilogue(MachineFunction &MF,
                    MachineBasicBlock &MBB) const override {}
  StackOffset getFrameIndexReference(const MachineFunction &MF, int FI,
                                     Register &FrameReg) const override;

protected:
  bool hasFPImpl(const MachineFunction &MF) const override { return false; }
};

} // end namespace vm::core

#endif // LLVM_LIB_TARGET_AMDGPU_R600FRAMELOWERING_H
