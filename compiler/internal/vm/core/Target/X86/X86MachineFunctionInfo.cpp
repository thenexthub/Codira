//===-- X86MachineFunctionInfo.cpp - X86 machine function info ------------===//
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

#include "X86MachineFunctionInfo.h"
#include "X86RegisterInfo.h"
#include "vm/core/CodeGen/MachineRegisterInfo.h"
#include "vm/core/CodeGen/TargetSubtargetInfo.h"

using namespace vm::core;

yaml::X86MachineFunctionInfo::X86MachineFunctionInfo(
    const toolchain::X86MachineFunctionInfo &MFI)
    : AMXProgModel(MFI.getAMXProgModel()) {}

void yaml::X86MachineFunctionInfo::mappingImpl(yaml::IO &YamlIO) {
  MappingTraits<X86MachineFunctionInfo>::mapping(YamlIO, *this);
}

MachineFunctionInfo *X86MachineFunctionInfo::clone(
    BumpPtrAllocator &Allocator, MachineFunction &DestMF,
    const DenseMap<MachineBasicBlock *, MachineBasicBlock *> &Src2DstMBB)
    const {
  return DestMF.cloneInfo<X86MachineFunctionInfo>(*this);
}

void X86MachineFunctionInfo::initializeBaseYamlFields(
    const yaml::X86MachineFunctionInfo &YamlMFI) {
  AMXProgModel = YamlMFI.AMXProgModel;
}

void X86MachineFunctionInfo::anchor() { }

void X86MachineFunctionInfo::setRestoreBasePointer(const MachineFunction *MF) {
  if (!RestoreBasePointerOffset) {
    const X86RegisterInfo *RegInfo = static_cast<const X86RegisterInfo *>(
      MF->getSubtarget().getRegisterInfo());
    unsigned SlotSize = RegInfo->getSlotSize();
    for (const MCPhysReg *CSR = MF->getRegInfo().getCalleeSavedRegs();
         unsigned Reg = *CSR; ++CSR) {
      if (X86::GR64RegClass.contains(Reg) || X86::GR32RegClass.contains(Reg))
        RestoreBasePointerOffset -= SlotSize;
    }
  }
}

