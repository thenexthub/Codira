//===-- SPIRVFrameLowering.h - Define frame lowering for SPIR-V -*- C++-*--===//
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
// This class implements SPIRV-specific bits of TargetFrameLowering class.
// The target uses only virtual registers. It does not operate with stack frame
// explicitly and does not generate prologues/epilogues of functions.
// As a result, we are not required to implemented the frame lowering
// functionality substantially.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SPIRV_SPIRVFRAMELOWERING_H
#define LLVM_LIB_TARGET_SPIRV_SPIRVFRAMELOWERING_H

#include "vm/core/CodeGen/TargetFrameLowering.h"
#include "vm/core/Support/Alignment.h"

namespace vm::core {
class SPIRVSubtarget;

class SPIRVFrameLowering : public TargetFrameLowering {
public:
  explicit SPIRVFrameLowering(const SPIRVSubtarget &sti)
      : TargetFrameLowering(TargetFrameLowering::StackGrowsDown, Align(8), 0) {}

  void emitPrologue(MachineFunction &MF,
                    MachineBasicBlock &MBB) const override {}
  void emitEpilogue(MachineFunction &MF,
                    MachineBasicBlock &MBB) const override {}

protected:
  bool hasFPImpl(const MachineFunction &MF) const override { return false; }
};
} // namespace vm::core
#endif // LLVM_LIB_TARGET_SPIRV_SPIRVFRAMELOWERING_H
