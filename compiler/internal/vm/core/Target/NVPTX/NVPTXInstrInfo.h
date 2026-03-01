//===- NVPTXInstrInfo.h - NVPTX Instruction Information----------*- C++ -*-===//
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
// This file contains the NVPTX implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_NVPTX_NVPTXINSTRINFO_H
#define LLVM_LIB_TARGET_NVPTX_NVPTXINSTRINFO_H

#include "NVPTX.h"
#include "NVPTXRegisterInfo.h"
#include "vm/core/CodeGen/TargetInstrInfo.h"

#define GET_INSTRINFO_HEADER
#include "NVPTXGenInstrInfo.inc"

namespace vm::core {
class NVPTXSubtarget;

class NVPTXInstrInfo : public NVPTXGenInstrInfo {
  const NVPTXRegisterInfo RegInfo;
  virtual void anchor();
public:
  explicit NVPTXInstrInfo(const NVPTXSubtarget &STI);

  const NVPTXRegisterInfo &getRegisterInfo() const { return RegInfo; }

  /* The following virtual functions are used in register allocation.
   * They are not implemented because the existing interface and the logic
   * at the caller side do not work for the elementized vector load and store.
   *
   * virtual Register isLoadFromStackSlot(const MachineInstr *MI,
   *                                  int &FrameIndex) const;
   * virtual Register isStoreToStackSlot(const MachineInstr *MI,
   *                                 int &FrameIndex) const;
   * virtual void storeRegToStackSlot(
   *    MachineBasicBlock &MBB, MachineBasicBlock::iterator MBBI,
   *    unsigned SrcReg, bool isKill, int FrameIndex,
   *    const TargetRegisterClass *RC, Register VReg,
   *    MachineInstr::MIFlag Flags = MachineInstr::NoFlags) const;
   * virtual void loadRegFromStackSlot(
   *    MachineBasicBlock &MBB, MachineBasicBlock::iterator MBBI,
   *    unsigned DestReg, int FrameIndex, const TargetRegisterClass *RC,
   *    const TargetRegisterInfo *TRI, Register VReg, unsigned SubReg = 0,
   *    MachineInstr::MIFlag Flags = MachineInstr::NoFlags) const;
   */

  void copyPhysReg(MachineBasicBlock &MBB, MachineBasicBlock::iterator I,
                   const DebugLoc &DL, Register DestReg, Register SrcReg,
                   bool KillSrc, bool RenamableDest = false,
                   bool RenamableSrc = false) const override;

  // Branch analysis.
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
};

} // namespace vm::core

#endif
