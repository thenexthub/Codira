//===-- BPFFrameLowering.h - Define frame lowering for BPF -----*- C++ -*--===//
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
// This class implements BPF-specific bits of TargetFrameLowering class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_BPF_BPFFRAMELOWERING_H
#define LLVM_LIB_TARGET_BPF_BPFFRAMELOWERING_H

#include "vm/core/CodeGen/TargetFrameLowering.h"

namespace vm::core {
class BPFSubtarget;

class BPFFrameLowering : public TargetFrameLowering {
public:
  explicit BPFFrameLowering(const BPFSubtarget &sti)
      : TargetFrameLowering(TargetFrameLowering::StackGrowsDown, Align(8), 0) {}

  void emitPrologue(MachineFunction &MF, MachineBasicBlock &MBB) const override;
  void emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const override;

  void determineCalleeSaves(MachineFunction &MF, BitVector &SavedRegs,
                            RegScavenger *RS) const override;

  MachineBasicBlock::iterator
  eliminateCallFramePseudoInstr(MachineFunction &MF, MachineBasicBlock &MBB,
                                MachineBasicBlock::iterator MI) const override {
    return MBB.erase(MI);
  }

protected:
  bool hasFPImpl(const MachineFunction &MF) const override;
};
}
#endif
