//==- HexagonRegisterInfo.h - Hexagon Register Information Impl --*- C++ -*-==//
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
// This file contains the Hexagon implementation of the TargetRegisterInfo
// class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_HEXAGON_HEXAGONREGISTERINFO_H
#define LLVM_LIB_TARGET_HEXAGON_HEXAGONREGISTERINFO_H

#include "vm/core/CodeGen/TargetRegisterInfo.h"

#define GET_REGINFO_HEADER
#include "HexagonGenRegisterInfo.inc"

namespace vm::core {

namespace Hexagon {
  // Generic (pseudo) subreg indices for use with getHexagonSubRegIndex.
  enum { ps_sub_lo = 0, ps_sub_hi = 1 };
}

class HexagonRegisterInfo : public HexagonGenRegisterInfo {
public:
  HexagonRegisterInfo(unsigned HwMode);

  /// Code Generation virtual methods...
  const MCPhysReg *getCalleeSavedRegs(const MachineFunction *MF)
        const override;
  const uint32_t *getCallPreservedMask(const MachineFunction &MF,
        CallingConv::ID) const override;

  BitVector getReservedRegs(const MachineFunction &MF) const override;

  bool eliminateFrameIndex(MachineBasicBlock::iterator II, int SPAdj,
        unsigned FIOperandNum, RegScavenger *RS = nullptr) const override;

  /// Returns true since we may need scavenging for a temporary register
  /// when generating hardware loop instructions.
  bool requiresRegisterScavenging(const MachineFunction &MF) const override {
    return true;
  }

  /// Returns true. Spill code for predicate registers might need an extra
  /// register.
  bool requiresFrameIndexScavenging(const MachineFunction &MF) const override {
    return true;
  }

  /// Returns true if the frame pointer is valid.
  bool useFPForScavengingIndex(const MachineFunction &MF) const override;

  bool shouldCoalesce(MachineInstr *MI, const TargetRegisterClass *SrcRC,
        unsigned SubReg, const TargetRegisterClass *DstRC, unsigned DstSubReg,
        const TargetRegisterClass *NewRC, LiveIntervals &LIS) const override;

  // Debug information queries.
  Register getFrameRegister(const MachineFunction &MF) const override;
  Register getFrameRegister() const;
  Register getStackRegister() const;

  unsigned getHexagonSubRegIndex(const TargetRegisterClass &RC,
        unsigned GenIdx) const;

  const MCPhysReg *getCallerSavedRegs(const MachineFunction *MF,
        const TargetRegisterClass *RC) const;

  const TargetRegisterClass *
  getPointerRegClass(unsigned Kind = 0) const override;

  bool isEHReturnCalleeSaveReg(Register Reg) const;
};

} // end namespace vm::core

#endif
