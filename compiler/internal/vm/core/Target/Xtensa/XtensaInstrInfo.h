//===-- XtensaInstrInfo.h - Xtensa Instruction Information ------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
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
// This file contains the Xtensa implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_XTENSA_XTENSAINSTRINFO_H
#define LLVM_LIB_TARGET_XTENSA_XTENSAINSTRINFO_H

#include "Xtensa.h"
#include "XtensaRegisterInfo.h"
#include "vm/core/CodeGen/TargetInstrInfo.h"
#include "vm/core/CodeGen/TargetRegisterInfo.h"

#define GET_INSTRINFO_HEADER

#include "XtensaGenInstrInfo.inc"

namespace vm::core {

class XtensaTargetMachine;
class XtensaSubtarget;
class XtensaInstrInfo : public XtensaGenInstrInfo {
  const XtensaRegisterInfo RI;
  const XtensaSubtarget &STI;

public:
  XtensaInstrInfo(const XtensaSubtarget &STI);

  void adjustStackPtr(MCRegister SP, int64_t Amount, MachineBasicBlock &MBB,
                      MachineBasicBlock::iterator I) const;

  unsigned getInstSizeInBytes(const MachineInstr &MI) const override;

  // Return the XtensaRegisterInfo, which this class owns.
  const XtensaRegisterInfo &getRegisterInfo() const { return RI; }

  Register isLoadFromStackSlot(const MachineInstr &MI,
                               int &FrameIndex) const override;

  Register isStoreToStackSlot(const MachineInstr &MI,
                              int &FrameIndex) const override;

  void copyPhysReg(MachineBasicBlock &MBB, MachineBasicBlock::iterator MBBI,
                   const DebugLoc &DL, Register DestReg, Register SrcReg,
                   bool KillSrc, bool RenamableDest = false,
                   bool RenamableSrc = false) const override;

  void storeRegToStackSlot(
      MachineBasicBlock &MBB, MachineBasicBlock::iterator MBBI, Register SrcReg,
      bool isKill, int FrameIndex, const TargetRegisterClass *RC, Register VReg,
      MachineInstr::MIFlag Flags = MachineInstr::NoFlags) const override;

  void loadRegFromStackSlot(
      MachineBasicBlock &MBB, MachineBasicBlock::iterator MBBI,
      Register DestReg, int FrameIdx, const TargetRegisterClass *RC,
      Register VReg, unsigned SubReg = 0,
      MachineInstr::MIFlag Flags = MachineInstr::NoFlags) const override;

  // Get the load and store opcodes for a given register class and offset.
  void getLoadStoreOpcodes(const TargetRegisterClass *RC, unsigned &LoadOpcode,
                           unsigned &StoreOpcode, int64_t offset) const;

  // Emit code before MBBI in MI to move immediate value Value into
  // physical register Reg.
  void loadImmediate(MachineBasicBlock &MBB, MachineBasicBlock::iterator MBBI,
                     MCRegister *Reg, int64_t Value) const;

  bool
  reverseBranchCondition(SmallVectorImpl<MachineOperand> &Cond) const override;

  MachineBasicBlock *getBranchDestBlock(const MachineInstr &MI) const override;

  bool isBranchOffsetInRange(unsigned BranchOpc,
                             int64_t BrOffset) const override;

  bool analyzeBranch(MachineBasicBlock &MBB, MachineBasicBlock *&TBB,
                     MachineBasicBlock *&FBB,
                     SmallVectorImpl<MachineOperand> &Cond,
                     bool AllowModify) const override;

  unsigned removeBranch(MachineBasicBlock &MBB,
                        int *BytesRemoved = nullptr) const override;

  unsigned insertBranch(MachineBasicBlock &MBB, MachineBasicBlock *TBB,
                        MachineBasicBlock *FBB, ArrayRef<MachineOperand> Cond,
                        const DebugLoc &DL,
                        int *BytesAdded = nullptr) const override;

  void insertIndirectBranch(MachineBasicBlock &MBB, MachineBasicBlock &DestBB,
                            MachineBasicBlock &RestoreBB, const DebugLoc &DL,
                            int64_t BrOffset = 0,
                            RegScavenger *RS = nullptr) const override;

  unsigned insertBranchAtInst(MachineBasicBlock &MBB,
                              MachineBasicBlock::iterator I,
                              MachineBasicBlock *TBB,
                              ArrayRef<MachineOperand> Cond, const DebugLoc &DL,
                              int *BytesAdded) const;

  unsigned insertConstBranchAtInst(MachineBasicBlock &MBB, MachineInstr *I,
                                   int64_t offset,
                                   ArrayRef<MachineOperand> Cond, DebugLoc DL,
                                   int *BytesAdded) const;

  // Return true if MI is a conditional or unconditional branch.
  // When returning true, set Cond to the mask of condition-code
  // values on which the instruction will branch, and set Target
  // to the operand that contains the branch target.  This target
  // can be a register or a basic block.
  bool isBranch(const MachineBasicBlock::iterator &MI,
                SmallVectorImpl<MachineOperand> &Cond,
                const MachineOperand *&Target) const;

  const XtensaSubtarget &getSubtarget() const { return STI; }
};
} // end namespace vm::core

#endif /* LLVM_LIB_TARGET_XTENSA_XTENSAINSTRINFO_H */
