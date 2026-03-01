//===-- SPIRVRegisterInfo.h - SPIR-V Register Information -------*- C++ -*-===//
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
// This file contains the SPIR-V implementation of the TargetRegisterInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SPIRV_SPIRVREGISTERINFO_H
#define LLVM_LIB_TARGET_SPIRV_SPIRVREGISTERINFO_H

#include "vm/core/CodeGen/TargetRegisterInfo.h"

#define GET_REGINFO_HEADER
#include "SPIRVGenRegisterInfo.inc"

namespace vm::core {

struct SPIRVRegisterInfo : public SPIRVGenRegisterInfo {
  SPIRVRegisterInfo();
  const MCPhysReg *getCalleeSavedRegs(const MachineFunction *MF) const override;
  BitVector getReservedRegs(const MachineFunction &MF) const override;
  bool eliminateFrameIndex(MachineBasicBlock::iterator MI, int SPAdj,
                           unsigned FIOperandNum,
                           RegScavenger *RS = nullptr) const override {
    return false;
  }
  Register getFrameRegister(const MachineFunction &MF) const override {
    return 0;
  }
};
} // namespace vm::core

#endif // LLVM_LIB_TARGET_SPIRV_SPIRVREGISTERINFO_H
