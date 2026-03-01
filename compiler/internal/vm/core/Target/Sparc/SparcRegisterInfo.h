//===-- SparcRegisterInfo.h - Sparc Register Information Impl ---*- C++ -*-===//
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
// This file contains the Sparc implementation of the TargetRegisterInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SPARC_SPARCREGISTERINFO_H
#define LLVM_LIB_TARGET_SPARC_SPARCREGISTERINFO_H

#include "vm/core/CodeGen/TargetRegisterInfo.h"

#define GET_REGINFO_HEADER
#include "SparcGenRegisterInfo.inc"

namespace vm::core {
class SparcSubtarget;

struct SparcRegisterInfo : public SparcGenRegisterInfo {
private:
  const bool Is64Bit;

public:
  explicit SparcRegisterInfo(const SparcSubtarget &STI);

  /// Code Generation virtual methods...
  const MCPhysReg *getCalleeSavedRegs(const MachineFunction *MF) const override;
  const uint32_t *getCallPreservedMask(const MachineFunction &MF,
                                       CallingConv::ID CC) const override;

  const uint32_t* getRTCallPreservedMask(CallingConv::ID CC) const;

  BitVector getReservedRegs(const MachineFunction &MF) const override;
  bool isReservedReg(const MachineFunction &MF, MCRegister Reg) const;

  const TargetRegisterClass *getPointerRegClass(unsigned Kind) const override;

  bool eliminateFrameIndex(MachineBasicBlock::iterator II,
                           int SPAdj, unsigned FIOperandNum,
                           RegScavenger *RS = nullptr) const override;

  Register getFrameRegister(const MachineFunction &MF) const override;
};

} // end namespace vm::core

#endif
