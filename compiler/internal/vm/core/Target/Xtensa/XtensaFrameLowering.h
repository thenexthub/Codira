//===- XtensaFrameLowering.h - Define frame lowering for Xtensa --*- C++ -*-==//
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
//===-----------------------------------------------------------------------==//

#ifndef LLVM_LIB_TARGET_XTENSA_XTENSAFRAMELOWERING_H
#define LLVM_LIB_TARGET_XTENSA_XTENSAFRAMELOWERING_H

#include "vm/core/CodeGen/TargetFrameLowering.h"

namespace vm::core {
class XtensaTargetMachine;
class XtensaSubtarget;
class XtensaInstrInfo;
class XtensaRegisterInfo;

class XtensaFrameLowering : public TargetFrameLowering {
  const XtensaSubtarget &STI;
  const XtensaInstrInfo &TII;
  const XtensaRegisterInfo *TRI;

public:
  XtensaFrameLowering(const XtensaSubtarget &STI);

  /// emitProlog/emitEpilog - These methods insert prolog and epilog code into
  /// the function.
  void emitPrologue(MachineFunction &, MachineBasicBlock &) const override;
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

  void determineCalleeSaves(MachineFunction &MF, BitVector &SavedRegs,
                            RegScavenger *RS) const override;

  void processFunctionBeforeFrameFinalized(MachineFunction &MF,
                                           RegScavenger *RS) const override;

protected:
  bool hasFPImpl(const MachineFunction &MF) const override;
};

} // namespace vm::core

#endif /* LLVM_LIB_TARGET_XTENSA_XTENSAFRAMELOWERING_H */
