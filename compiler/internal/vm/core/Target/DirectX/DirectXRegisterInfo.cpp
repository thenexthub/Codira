//===-- DirectXRegisterInfo.cpp - RegisterInfo for DirectX -*- C++ ------*-===//
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
// This file defines the DirectX specific subclass of TargetRegisterInfo.
//
//===----------------------------------------------------------------------===//

#include "DirectXRegisterInfo.h"
#include "DirectXFrameLowering.h"
#include "MCTargetDesc/DirectXMCTargetDesc.h"
#include "vm/core/CodeGen/MachineFunction.h"
#include "vm/core/CodeGen/TargetSubtargetInfo.h"

#define GET_REGINFO_TARGET_DESC
#include "DirectXGenRegisterInfo.inc"

using namespace vm::core;

DirectXRegisterInfo::~DirectXRegisterInfo() {}

const MCPhysReg *
DirectXRegisterInfo::getCalleeSavedRegs(const MachineFunction *MF) const {
  return nullptr;
}
BitVector
DirectXRegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  return BitVector(getNumRegs());
}

bool DirectXRegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator II,
                                              int SPAdj, unsigned FIOperandNum,
                                              RegScavenger *RS) const {
  return false;
}

// Debug information queries.
Register
DirectXRegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  return Register();
}
