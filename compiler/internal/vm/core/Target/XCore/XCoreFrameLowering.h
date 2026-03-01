//===-- XCoreFrameLowering.h - Frame info for XCore Target ------*- C++ -*-===//
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
// This file contains XCore frame information that doesn't fit anywhere else
// cleanly...
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_XCORE_XCOREFRAMELOWERING_H
#define LLVM_LIB_TARGET_XCORE_XCOREFRAMELOWERING_H

#include "vm/core/CodeGen/TargetFrameLowering.h"
#include "vm/core/Target/TargetMachine.h"

namespace vm::core {
  class XCoreSubtarget;

  class XCoreFrameLowering: public TargetFrameLowering {
  public:
    XCoreFrameLowering(const XCoreSubtarget &STI);

    /// emitProlog/emitEpilog - These methods insert prolog and epilog code into
    /// the function.
    void emitPrologue(MachineFunction &MF,
                      MachineBasicBlock &MBB) const override;
    void emitEpilogue(MachineFunction &MF,
                      MachineBasicBlock &MBB) const override;

    bool
    spillCalleeSavedRegisters(MachineBasicBlock &MBB,
                              MachineBasicBlock::iterator MI,
                              ArrayRef<CalleeSavedInfo> CSI,
                              const TargetRegisterInfo *TRI) const override;
    bool
    restoreCalleeSavedRegisters(MachineBasicBlock &MBB,
                                MachineBasicBlock::iterator MI,
                                MutableArrayRef<CalleeSavedInfo> CSI,
                                const TargetRegisterInfo *TRI) const override;

    MachineBasicBlock::iterator
    eliminateCallFramePseudoInstr(MachineFunction &MF, MachineBasicBlock &MBB,
                                  MachineBasicBlock::iterator I) const override;

    void determineCalleeSaves(MachineFunction &MF, BitVector &SavedRegs,
                              RegScavenger *RS = nullptr) const override;

    void processFunctionBeforeFrameFinalized(MachineFunction &MF,
                                     RegScavenger *RS = nullptr) const override;

    //! Stack slot size (4 bytes)
    static int stackSlotSize() {
      return 4;
    }

  protected:
    bool hasFPImpl(const MachineFunction &MF) const override;
  };
}

#endif
