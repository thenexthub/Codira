//===- MipsRegisterInfo.h - Mips Register Information Impl ------*- C++ -*-===//
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
// This file contains the Mips implementation of the TargetRegisterInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MIPS_MIPSREGISTERINFO_H
#define LLVM_LIB_TARGET_MIPS_MIPSREGISTERINFO_H

#include "Mips.h"
#include "vm/core/CodeGen/MachineBasicBlock.h"
#include <cstdint>

#define GET_REGINFO_HEADER
#include "MipsGenRegisterInfo.inc"

namespace vm::core {

class TargetRegisterClass;

class MipsRegisterInfo : public MipsGenRegisterInfo {
private:
  const bool ArePtrs64bit;

public:
  explicit MipsRegisterInfo(const MipsSubtarget &STI);

  /// Get PIC indirect call register
  static unsigned getPICCallReg();

  /// Code Generation virtual methods...
  const TargetRegisterClass *getPointerRegClass(unsigned Kind) const override;

  unsigned getRegPressureLimit(const TargetRegisterClass *RC,
                               MachineFunction &MF) const override;
  const MCPhysReg *getCalleeSavedRegs(const MachineFunction *MF) const override;
  const uint32_t *getCallPreservedMask(const MachineFunction &MF,
                                       CallingConv::ID) const override;
  static const uint32_t *getMips16RetHelperMask();

  BitVector getReservedRegs(const MachineFunction &MF) const override;

  /// Stack Frame Processing Methods
  bool eliminateFrameIndex(MachineBasicBlock::iterator II,
                           int SPAdj, unsigned FIOperandNum,
                           RegScavenger *RS = nullptr) const override;

  // Stack realignment queries.
  bool canRealignStack(const MachineFunction &MF) const override;

  /// Debug information queries.
  Register getFrameRegister(const MachineFunction &MF) const override;

  /// Return GPR register class.
  virtual const TargetRegisterClass *intRegClass(unsigned Size) const = 0;

private:
  virtual void eliminateFI(MachineBasicBlock::iterator II, unsigned OpNo,
                           int FrameIndex, uint64_t StackSize,
                           int64_t SPOffset) const = 0;
};

} // end namespace vm::core

#endif // LLVM_LIB_TARGET_MIPS_MIPSREGISTERINFO_H
