//===-- CSKYInstrInfo.h - CSKY Instruction Information --------*- C++ -*---===//
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
// This file contains the CSKY implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_CSKY_CSKYINSTRINFO_H
#define LLVM_LIB_TARGET_CSKY_CSKYINSTRINFO_H

#include "MCTargetDesc/CSKYMCTargetDesc.h"
#include "vm/core/CodeGen/TargetInstrInfo.h"

#define GET_INSTRINFO_HEADER
#include "CSKYGenInstrInfo.inc"

namespace vm::core {

class CSKYRegisterInfo;
class CSKYSubtarget;

class CSKYInstrInfo : public CSKYGenInstrInfo {
  bool v2sf;
  bool v2df;
  bool v3sf;
  bool v3df;

protected:
  const CSKYSubtarget &STI;

public:
  CSKYInstrInfo(const CSKYSubtarget &STI, const CSKYRegisterInfo &RI);

  Register isLoadFromStackSlot(const MachineInstr &MI,
                               int &FrameIndex) const override;
  Register isStoreToStackSlot(const MachineInstr &MI,
                              int &FrameIndex) const override;

  void storeRegToStackSlot(
      MachineBasicBlock &MBB, MachineBasicBlock::iterator MI, Register SrcReg,
      bool IsKill, int FrameIndex, const TargetRegisterClass *RC, Register VReg,
      MachineInstr::MIFlag Flags = MachineInstr::NoFlags) const override;

  void loadRegFromStackSlot(
      MachineBasicBlock &MBB, MachineBasicBlock::iterator MI, Register DestReg,
      int FrameIndex, const TargetRegisterClass *RC, Register VReg,
      unsigned SubReg = 0,
      MachineInstr::MIFlag Flags = MachineInstr::NoFlags) const override;

  void copyPhysReg(MachineBasicBlock &MBB, MachineBasicBlock::iterator MI,
                   const DebugLoc &DL, Register DestReg, Register SrcReg,
                   bool KillSrc, bool RenamableDest = false,
                   bool RenamableSrc = false) const override;

  unsigned insertBranch(MachineBasicBlock &MBB, MachineBasicBlock *TBB,
                        MachineBasicBlock *FBB, ArrayRef<MachineOperand> Cond,
                        const DebugLoc &DL,
                        int *BytesAdded = nullptr) const override;

  bool analyzeBranch(MachineBasicBlock &MBB, MachineBasicBlock *&TBB,
                     MachineBasicBlock *&FBB,
                     SmallVectorImpl<MachineOperand> &Cond,
                     bool AllowModify = false) const override;

  unsigned removeBranch(MachineBasicBlock &MBB,
                        int *BytesRemoved = nullptr) const override;

  bool
  reverseBranchCondition(SmallVectorImpl<MachineOperand> &Cond) const override;

  MachineBasicBlock *getBranchDestBlock(const MachineInstr &MI) const override;

  unsigned getInstSizeInBytes(const MachineInstr &MI) const override;

  Register getGlobalBaseReg(MachineFunction &MF) const;

  // Materializes the given integer Val into DstReg.
  Register movImm(MachineBasicBlock &MBB, MachineBasicBlock::iterator MBBI,
                  const DebugLoc &DL, uint64_t Val,
                  MachineInstr::MIFlag Flag = MachineInstr::NoFlags) const;
};

} // namespace vm::core

#endif // LLVM_LIB_TARGET_CSKY_CSKYINSTRINFO_H
