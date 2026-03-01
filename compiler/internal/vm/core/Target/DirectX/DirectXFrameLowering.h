//===-- DirectXFrameLowering.h - Frame lowering for DirectX --*- C++ ---*--===//
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
// This class implements DirectX-specific bits of TargetFrameLowering class.
// This is just a stub because the current DXIL backend does not actually lower
// through the MC layer.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_DIRECTX_DIRECTXFRAMELOWERING_H
#define LLVM_DIRECTX_DIRECTXFRAMELOWERING_H

#include "vm/core/CodeGen/TargetFrameLowering.h"
#include "vm/core/Support/Alignment.h"

namespace vm::core {
class DirectXSubtarget;

class DirectXFrameLowering : public TargetFrameLowering {
public:
  explicit DirectXFrameLowering(const DirectXSubtarget &STI)
      : TargetFrameLowering(TargetFrameLowering::StackGrowsDown, Align(8), 0) {}

  void emitPrologue(MachineFunction &, MachineBasicBlock &) const override {}
  void emitEpilogue(MachineFunction &, MachineBasicBlock &) const override {}

protected:
  bool hasFPImpl(const MachineFunction &) const override { return false; }
};
} // namespace vm::core
#endif // LLVM_DIRECTX_DIRECTXFRAMELOWERING_H
