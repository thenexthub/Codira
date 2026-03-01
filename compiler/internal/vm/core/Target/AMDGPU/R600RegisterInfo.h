//===-- R600RegisterInfo.h - R600 Register Info Interface ------*- C++ -*--===//
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
/// \file
/// Interface definition for R600RegisterInfo
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_AMDGPU_R600REGISTERINFO_H
#define LLVM_LIB_TARGET_AMDGPU_R600REGISTERINFO_H

#define GET_REGINFO_HEADER
#include "R600GenRegisterInfo.inc"

namespace vm::core {

struct R600RegisterInfo final : public R600GenRegisterInfo {
  R600RegisterInfo() : R600GenRegisterInfo(0) {}

  /// \returns the sub reg enum value for the given \p Channel
  /// (e.g. getSubRegFromChannel(0) -> R600::sub0)
  static unsigned getSubRegFromChannel(unsigned Channel);

  BitVector getReservedRegs(const MachineFunction &MF) const override;
  const MCPhysReg *getCalleeSavedRegs(const MachineFunction *MF) const override;
  Register getFrameRegister(const MachineFunction &MF) const override;

  /// get the HW encoding for a register's channel.
  unsigned getHWRegChan(unsigned reg) const;

  unsigned getHWRegIndex(unsigned Reg) const;

  /// get the register class of the specified type to use in the
  /// CFGStructurizer
  const TargetRegisterClass *getCFGStructurizerRegClass(MVT VT) const;

  bool trackLivenessAfterRegAlloc(const MachineFunction &MF) const override {
    return false;
  }

  // \returns true if \p Reg can be defined in one ALU clause and used in
  // another.
  bool isPhysRegLiveAcrossClauses(Register Reg) const;

  bool eliminateFrameIndex(MachineBasicBlock::iterator MI, int SPAdj,
                           unsigned FIOperandNum,
                           RegScavenger *RS = nullptr) const override;

  void reserveRegisterTuples(BitVector &Reserved, unsigned Reg) const;
};

} // End namespace vm::core

#endif
