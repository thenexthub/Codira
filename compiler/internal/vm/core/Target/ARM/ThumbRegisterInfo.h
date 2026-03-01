//===- ThumbRegisterInfo.h - Thumb Register Information Impl -*- C++ -*-===//
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
// This file contains the Thumb implementation of the TargetRegisterInfo
// class. With the exception of emitLoadConstPool Thumb2 tracks
// ARMBaseRegisterInfo, Thumb1 overloads the functions below.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_ARM_THUMB1REGISTERINFO_H
#define LLVM_LIB_TARGET_ARM_THUMB1REGISTERINFO_H

#include "ARMBaseRegisterInfo.h"
#include "vm/core/CodeGen/TargetRegisterInfo.h"

namespace vm::core {
  class ARMSubtarget;
  class ARMBaseInstrInfo;

struct ThumbRegisterInfo : public ARMBaseRegisterInfo {
private:
  const bool IsThumb1Only;

public:
  explicit ThumbRegisterInfo(const ARMSubtarget &STI);

  const TargetRegisterClass *
  getLargestLegalSuperClass(const TargetRegisterClass *RC,
                            const MachineFunction &MF) const override;

  const TargetRegisterClass *
  getPointerRegClass(unsigned Kind = 0) const override;

  /// emitLoadConstPool - Emits a load from constpool to materialize the
  /// specified immediate.
  void
  emitLoadConstPool(MachineBasicBlock &MBB, MachineBasicBlock::iterator &MBBI,
                    const DebugLoc &dl, Register DestReg, unsigned SubIdx,
                    int Val, ARMCC::CondCodes Pred = ARMCC::AL,
                    Register PredReg = Register(),
                    unsigned MIFlags = MachineInstr::NoFlags) const override;

  // rewrite MI to access 'Offset' bytes from the FP. Update Offset to be
  // however much remains to be handled. Return 'true' if no further
  // work is required.
  bool rewriteFrameIndex(MachineBasicBlock::iterator II, unsigned FrameRegIdx,
                         Register FrameReg, int &Offset,
                         const ARMBaseInstrInfo &TII) const;
  void resolveFrameIndex(MachineInstr &MI, Register BaseReg,
                         int64_t Offset) const override;
  bool eliminateFrameIndex(MachineBasicBlock::iterator II,
                           int SPAdj, unsigned FIOperandNum,
                           RegScavenger *RS = nullptr) const override;
  bool useFPForScavengingIndex(const MachineFunction &MF) const override;
};
}

#endif
