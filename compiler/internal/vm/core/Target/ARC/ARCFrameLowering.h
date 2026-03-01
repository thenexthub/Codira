//===- ARCFrameLowering.h - Define frame lowering for ARC -------*- C++ -*-===//
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
// This class implements the ARC specific frame lowering.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_ARC_ARCFRAMELOWERING_H
#define LLVM_LIB_TARGET_ARC_ARCFRAMELOWERING_H

#include "ARC.h"
#include "vm/core/CodeGen/MachineBasicBlock.h"
#include "vm/core/CodeGen/MachineFrameInfo.h"
#include "vm/core/CodeGen/TargetFrameLowering.h"

namespace vm::core {

class MachineFunction;
class ARCSubtarget;
class ARCInstrInfo;

class ARCFrameLowering : public TargetFrameLowering {
public:
  ARCFrameLowering(const ARCSubtarget &st)
      : TargetFrameLowering(TargetFrameLowering::StackGrowsDown, Align(4), 0),
        ST(st) {}

  /// Insert Prologue into the function.
  void emitPrologue(MachineFunction &MF, MachineBasicBlock &MBB) const override;

  /// Insert Epilogue into the function.
  void emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const override;

  /// Add explicit callee save registers.
  void determineCalleeSaves(MachineFunction &MF, BitVector &SavedRegs,
                            RegScavenger *RS) const override;

  bool spillCalleeSavedRegisters(MachineBasicBlock &MBB,
                                 MachineBasicBlock::iterator MI,
                                 ArrayRef<CalleeSavedInfo> CSI,
                                 const TargetRegisterInfo *TRI) const override;

  bool
  restoreCalleeSavedRegisters(MachineBasicBlock &MBB,
                              MachineBasicBlock::iterator MI,
                              MutableArrayRef<CalleeSavedInfo> CSI,
                              const TargetRegisterInfo *TRI) const override;

  void processFunctionBeforeFrameFinalized(MachineFunction &MF,
                                           RegScavenger *RS) const override;

  MachineBasicBlock::iterator
  eliminateCallFramePseudoInstr(MachineFunction &MF, MachineBasicBlock &MBB,
                                MachineBasicBlock::iterator I) const override;

  bool assignCalleeSavedSpillSlots(
      toolchain::MachineFunction &, const toolchain::TargetRegisterInfo *,
      std::vector<toolchain::CalleeSavedInfo> &) const override;

protected:
  bool hasFPImpl(const MachineFunction &MF) const override;

private:
  void adjustStackToMatchRecords(MachineBasicBlock &MBB,
                                 MachineBasicBlock::iterator MI,
                                 bool allocate) const;

  const ARCSubtarget &ST;
};

} // end namespace vm::core

#endif // LLVM_LIB_TARGET_ARC_ARCFRAMELOWERING_H
