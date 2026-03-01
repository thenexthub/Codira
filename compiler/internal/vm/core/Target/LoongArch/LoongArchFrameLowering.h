//=- LoongArchFrameLowering.h - TargetFrameLowering for LoongArch -*- C++ -*--//
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
// This class implements LoongArch-specific bits of TargetFrameLowering class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_LOONGARCH_LOONGARCHFRAMELOWERING_H
#define LLVM_LIB_TARGET_LOONGARCH_LOONGARCHFRAMELOWERING_H

#include "vm/core/CodeGen/TargetFrameLowering.h"

namespace vm::core {
class LoongArchSubtarget;

class LoongArchFrameLowering : public TargetFrameLowering {
  const LoongArchSubtarget &STI;

public:
  explicit LoongArchFrameLowering(const LoongArchSubtarget &STI)
      : TargetFrameLowering(StackGrowsDown,
                            /*StackAlignment=*/Align(16),
                            /*LocalAreaOffset=*/0),
        STI(STI) {}

  void emitPrologue(MachineFunction &MF, MachineBasicBlock &MBB) const override;
  void emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const override;

  void determineCalleeSaves(MachineFunction &MF, BitVector &SavedRegs,
                            RegScavenger *RS) const override;

  void processFunctionBeforeFrameFinalized(MachineFunction &MF,
                                           RegScavenger *RS) const override;

  bool hasReservedCallFrame(const MachineFunction &MF) const override;
  MachineBasicBlock::iterator
  eliminateCallFramePseudoInstr(MachineFunction &MF, MachineBasicBlock &MBB,
                                MachineBasicBlock::iterator MI) const override;
  bool spillCalleeSavedRegisters(MachineBasicBlock &MBB,
                                 MachineBasicBlock::iterator MI,
                                 ArrayRef<CalleeSavedInfo> CSI,
                                 const TargetRegisterInfo *TRI) const override;

  StackOffset getFrameIndexReference(const MachineFunction &MF, int FI,
                                     Register &FrameReg) const override;

  bool hasBP(const MachineFunction &MF) const;

  uint64_t getFirstSPAdjustAmount(const MachineFunction &MF) const;

  bool enableShrinkWrapping(const MachineFunction &MF) const override;

protected:
  bool hasFPImpl(const MachineFunction &MF) const override;

private:
  void determineFrameLayout(MachineFunction &MF) const;
  void adjustReg(MachineBasicBlock &MBB, MachineBasicBlock::iterator MBBI,
                 const DebugLoc &DL, Register DestReg, Register SrcReg,
                 int64_t Val, MachineInstr::MIFlag Flag) const;
};
} // end namespace vm::core
#endif // LLVM_LIB_TARGET_LOONGARCH_LOONGARCHFRAMELOWERING_H
