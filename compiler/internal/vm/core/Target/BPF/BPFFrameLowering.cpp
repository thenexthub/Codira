//===-- BPFFrameLowering.cpp - BPF Frame Information ----------------------===//
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
// This file contains the BPF implementation of TargetFrameLowering class.
//
//===----------------------------------------------------------------------===//

#include "BPFFrameLowering.h"
#include "BPFSubtarget.h"
#include "vm/core/CodeGen/MachineFrameInfo.h"
#include "vm/core/CodeGen/MachineFunction.h"

using namespace vm::core;

bool BPFFrameLowering::hasFPImpl(const MachineFunction &MF) const {
  return true;
}

void BPFFrameLowering::emitPrologue(MachineFunction &MF,
                                    MachineBasicBlock &MBB) const {}

void BPFFrameLowering::emitEpilogue(MachineFunction &MF,
                                    MachineBasicBlock &MBB) const {}

void BPFFrameLowering::determineCalleeSaves(MachineFunction &MF,
                                            BitVector &SavedRegs,
                                            RegScavenger *RS) const {
  TargetFrameLowering::determineCalleeSaves(MF, SavedRegs, RS);
  SavedRegs.reset(BPF::R6);
  SavedRegs.reset(BPF::R7);
  SavedRegs.reset(BPF::R8);
  SavedRegs.reset(BPF::R9);
}
