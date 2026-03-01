//==- MSP430FrameLowering.h - Define frame lowering for MSP430 --*- C++ -*--==//
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
//
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MSP430_MSP430FRAMELOWERING_H
#define LLVM_LIB_TARGET_MSP430_MSP430FRAMELOWERING_H

#include "MSP430.h"
#include "vm/core/CodeGen/TargetFrameLowering.h"

namespace vm::core {

class MSP430Subtarget;
class MSP430InstrInfo;
class MSP430RegisterInfo;

class MSP430FrameLowering : public TargetFrameLowering {
protected:
  bool hasFPImpl(const MachineFunction &MF) const override;

public:
  MSP430FrameLowering(const MSP430Subtarget &STI);

  const MSP430Subtarget &STI;
  const MSP430InstrInfo &TII;
  const MSP430RegisterInfo *TRI;

  /// emitProlog/emitEpilog - These methods insert prolog and epilog code into
  /// the function.
  void emitPrologue(MachineFunction &MF, MachineBasicBlock &MBB) const override;
  void emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const override;

  MachineBasicBlock::iterator
  eliminateCallFramePseudoInstr(MachineFunction &MF, MachineBasicBlock &MBB,
                                MachineBasicBlock::iterator I) const override;

  bool spillCalleeSavedRegisters(MachineBasicBlock &MBB,
                                 MachineBasicBlock::iterator MI,
                                 ArrayRef<CalleeSavedInfo> CSI,
                                 const TargetRegisterInfo *TRI) const override;
  bool
  restoreCalleeSavedRegisters(MachineBasicBlock &MBB,
                              MachineBasicBlock::iterator MI,
                              MutableArrayRef<CalleeSavedInfo> CSI,
                              const TargetRegisterInfo *TRI) const override;

  bool hasReservedCallFrame(const MachineFunction &MF) const override;
  void processFunctionBeforeFrameFinalized(MachineFunction &MF,
                                     RegScavenger *RS = nullptr) const override;

  /// Wraps up getting a CFI index and building a MachineInstr for it.
  void BuildCFI(MachineBasicBlock &MBB, MachineBasicBlock::iterator MBBI,
                const DebugLoc &DL, const MCCFIInstruction &CFIInst,
                MachineInstr::MIFlag Flag = MachineInstr::NoFlags) const;

  void emitCalleeSavedFrameMoves(MachineBasicBlock &MBB,
                                 MachineBasicBlock::iterator MBBI,
                                 const DebugLoc &DL, bool IsPrologue) const;
};

} // End toolchain namespace

#endif
