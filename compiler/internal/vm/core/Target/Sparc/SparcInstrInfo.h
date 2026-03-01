//===-- SparcInstrInfo.h - Sparc Instruction Information --------*- C++ -*-===//
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
// This file contains the Sparc implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SPARC_SPARCINSTRINFO_H
#define LLVM_LIB_TARGET_SPARC_SPARCINSTRINFO_H

#include "SparcRegisterInfo.h"
#include "vm/core/CodeGen/TargetInstrInfo.h"

#define GET_INSTRINFO_HEADER
#include "SparcGenInstrInfo.inc"

namespace vm::core {

class SparcSubtarget;

/// SPII - This namespace holds all of the target specific flags that
/// instruction info tracks.
///
namespace SPII {
  enum {
    Pseudo = (1<<0),
    Load = (1<<1),
    Store = (1<<2),
    DelaySlot = (1<<3)
  };
}

class SparcInstrInfo : public SparcGenInstrInfo {
  const SparcRegisterInfo RI;
  const SparcSubtarget& Subtarget;
  virtual void anchor();
public:
  explicit SparcInstrInfo(const SparcSubtarget &ST);

  /// getRegisterInfo - TargetInstrInfo is a superset of MRegister info.  As
  /// such, whenever a client has an instance of instruction info, it should
  /// always be able to get register info as well (through this method).
  ///
  const SparcRegisterInfo &getRegisterInfo() const { return RI; }

  /// isLoadFromStackSlot - If the specified machine instruction is a direct
  /// load from a stack slot, return the virtual or physical register number of
  /// the destination along with the FrameIndex of the loaded stack slot.  If
  /// not, return 0.  This predicate must return 0 if the instruction has
  /// any side effects other than loading from the stack slot.
  Register isLoadFromStackSlot(const MachineInstr &MI,
                               int &FrameIndex) const override;

  /// isStoreToStackSlot - If the specified machine instruction is a direct
  /// store to a stack slot, return the virtual or physical register number of
  /// the source reg along with the FrameIndex of the loaded stack slot.  If
  /// not, return 0.  This predicate must return 0 if the instruction has
  /// any side effects other than storing to the stack slot.
  Register isStoreToStackSlot(const MachineInstr &MI,
                              int &FrameIndex) const override;

  MachineBasicBlock *getBranchDestBlock(const MachineInstr &MI) const override;

  bool analyzeBranch(MachineBasicBlock &MBB, MachineBasicBlock *&TBB,
                     MachineBasicBlock *&FBB,
                     SmallVectorImpl<MachineOperand> &Cond,
                     bool AllowModify = false) const override;

  unsigned removeBranch(MachineBasicBlock &MBB,
                        int *BytesRemoved = nullptr) const override;

  unsigned insertBranch(MachineBasicBlock &MBB, MachineBasicBlock *TBB,
                        MachineBasicBlock *FBB, ArrayRef<MachineOperand> Cond,
                        const DebugLoc &DL,
                        int *BytesAdded = nullptr) const override;

  bool
  reverseBranchCondition(SmallVectorImpl<MachineOperand> &Cond) const override;

  /// Determine if the branch target is in range.
  bool isBranchOffsetInRange(unsigned BranchOpc, int64_t Offset) const override;

  void copyPhysReg(MachineBasicBlock &MBB, MachineBasicBlock::iterator I,
                   const DebugLoc &DL, Register DestReg, Register SrcReg,
                   bool KillSrc, bool RenamableDest = false,
                   bool RenamableSrc = false) const override;

  void storeRegToStackSlot(
      MachineBasicBlock &MBB, MachineBasicBlock::iterator MBBI, Register SrcReg,
      bool isKill, int FrameIndex, const TargetRegisterClass *RC, Register VReg,
      MachineInstr::MIFlag Flags = MachineInstr::NoFlags) const override;

  void loadRegFromStackSlot(
      MachineBasicBlock &MBB, MachineBasicBlock::iterator MBBI,
      Register DestReg, int FrameIndex, const TargetRegisterClass *RC,
      Register VReg, unsigned SubReg = 0,
      MachineInstr::MIFlag Flags = MachineInstr::NoFlags) const override;

  Register getGlobalBaseReg(MachineFunction *MF) const;

  /// GetInstSize - Return the number of bytes of code the specified
  /// instruction may be.  This returns the maximum number of bytes.
  unsigned getInstSizeInBytes(const MachineInstr &MI) const override;

  bool analyzeCompare(const MachineInstr &MI, Register &SrcReg,
                      Register &SrcReg2, int64_t &CmpMask,
                      int64_t &CmpValue) const override;

  bool optimizeCompareInstr(MachineInstr &CmpInstr, Register SrcReg,
                            Register SrcReg2, int64_t CmpMask, int64_t CmpValue,
                            const MachineRegisterInfo *MRI) const override;

  // Lower pseudo instructions after register allocation.
  bool expandPostRAPseudo(MachineInstr &MI) const override;
};

}

#endif
