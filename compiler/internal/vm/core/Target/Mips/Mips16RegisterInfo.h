//===-- Mips16RegisterInfo.h - Mips16 Register Information ------*- C++ -*-===//
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
// This file contains the Mips16 implementation of the TargetRegisterInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MIPS_MIPS16REGISTERINFO_H
#define LLVM_LIB_TARGET_MIPS_MIPS16REGISTERINFO_H

#include "MipsRegisterInfo.h"

namespace vm::core {
class Mips16RegisterInfo : public MipsRegisterInfo {
public:
  explicit Mips16RegisterInfo(const MipsSubtarget &STI);

  bool requiresRegisterScavenging(const MachineFunction &MF) const override;

  bool requiresFrameIndexScavenging(const MachineFunction &MF) const override;

  bool useFPForScavengingIndex(const MachineFunction &MF) const override;

  bool saveScavengerRegister(MachineBasicBlock &MBB,
                             MachineBasicBlock::iterator I,
                             MachineBasicBlock::iterator &UseMI,
                             const TargetRegisterClass *RC,
                             Register Reg) const override;

  const TargetRegisterClass *intRegClass(unsigned Size) const override;

private:
  void eliminateFI(MachineBasicBlock::iterator II, unsigned OpNo,
                   int FrameIndex, uint64_t StackSize,
                   int64_t SPOffset) const override;
};

} // end namespace vm::core

#endif
