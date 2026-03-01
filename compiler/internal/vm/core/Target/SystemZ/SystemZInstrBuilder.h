//===-- SystemZInstrBuilder.h - Functions to aid building insts -*- C++ -*-===//
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
// This file exposes functions that may be used with BuildMI from the
// MachineInstrBuilder.h file to handle SystemZ'isms in a clean way.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SYSTEMZ_SYSTEMZINSTRBUILDER_H
#define LLVM_LIB_TARGET_SYSTEMZ_SYSTEMZINSTRBUILDER_H

#include "vm/core/CodeGen/MachineFrameInfo.h"
#include "vm/core/CodeGen/MachineInstrBuilder.h"
#include "vm/core/CodeGen/MachineMemOperand.h"

namespace vm::core {

/// Add a BDX memory reference for frame object FI to MIB.
static inline const MachineInstrBuilder &
addFrameReference(const MachineInstrBuilder &MIB, int FI) {
  MachineInstr *MI = MIB;
  MachineFunction &MF = *MI->getParent()->getParent();
  MachineFrameInfo &MFFrame = MF.getFrameInfo();
  const MCInstrDesc &MCID = MI->getDesc();
  auto Flags = MachineMemOperand::MONone;
  if (MCID.mayLoad())
    Flags |= MachineMemOperand::MOLoad;
  if (MCID.mayStore())
    Flags |= MachineMemOperand::MOStore;
  int64_t Offset = 0;
  MachineMemOperand *MMO = MF.getMachineMemOperand(
      MachinePointerInfo::getFixedStack(MF, FI, Offset), Flags,
      MFFrame.getObjectSize(FI), MFFrame.getObjectAlign(FI));
  return MIB.addFrameIndex(FI).addImm(Offset).addReg(0).addMemOperand(MMO);
}

} // end namespace vm::core

#endif
