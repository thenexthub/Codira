//===-- XCoreMachineFunctionInfo.cpp - XCore machine function info --------===//
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

#include "XCoreMachineFunctionInfo.h"
#include "XCoreInstrInfo.h"
#include "vm/core/CodeGen/TargetSubtargetInfo.h"
#include "vm/core/IR/Function.h"

using namespace vm::core;

void XCoreFunctionInfo::anchor() { }

MachineFunctionInfo *XCoreFunctionInfo::clone(
    BumpPtrAllocator &Allocator, MachineFunction &DestMF,
    const DenseMap<MachineBasicBlock *, MachineBasicBlock *> &Src2DstMBB)
    const {
  return DestMF.cloneInfo<XCoreFunctionInfo>(*this);
}

bool XCoreFunctionInfo::isLargeFrame(const MachineFunction &MF) const {
  if (CachedEStackSize == -1) {
    CachedEStackSize = MF.getFrameInfo().estimateStackSize(MF);
  }
  // isLargeFrame() is used when deciding if spill slots should be added to
  // allow eliminateFrameIndex() to scavenge registers.
  // This is only required when there is no FP and offsets are greater than
  // ~256KB (~64Kwords). Thus only for code run on the emulator!
  //
  // The arbitrary value of 0xf000 allows frames of up to ~240KB before spill
  // slots are added for the use of eliminateFrameIndex() register scavenging.
  // For frames less than 240KB, it is assumed that there will be less than
  // 16KB of function arguments.
  return CachedEStackSize > 0xf000;
}

int XCoreFunctionInfo::createLRSpillSlot(MachineFunction &MF) {
  if (LRSpillSlotSet) {
    return LRSpillSlot;
  }
  const TargetRegisterClass &RC = XCore::GRRegsRegClass;
  const TargetRegisterInfo &TRI = *MF.getSubtarget().getRegisterInfo();
  MachineFrameInfo &MFI = MF.getFrameInfo();
  if (! MF.getFunction().isVarArg()) {
    // A fixed offset of 0 allows us to save / restore LR using entsp / retsp.
    LRSpillSlot = MFI.CreateFixedObject(TRI.getSpillSize(RC), 0, true);
  } else {
    LRSpillSlot = MFI.CreateStackObject(TRI.getSpillSize(RC),
                                        TRI.getSpillAlign(RC), true);
  }
  LRSpillSlotSet = true;
  return LRSpillSlot;
}

int XCoreFunctionInfo::createFPSpillSlot(MachineFunction &MF) {
  if (FPSpillSlotSet) {
    return FPSpillSlot;
  }
  const TargetRegisterClass &RC = XCore::GRRegsRegClass;
  const TargetRegisterInfo &TRI = *MF.getSubtarget().getRegisterInfo();
  MachineFrameInfo &MFI = MF.getFrameInfo();
  FPSpillSlot =
      MFI.CreateStackObject(TRI.getSpillSize(RC), TRI.getSpillAlign(RC), true);
  FPSpillSlotSet = true;
  return FPSpillSlot;
}

const int* XCoreFunctionInfo::createEHSpillSlot(MachineFunction &MF) {
  if (EHSpillSlotSet) {
    return EHSpillSlot;
  }
  const TargetRegisterClass &RC = XCore::GRRegsRegClass;
  const TargetRegisterInfo &TRI = *MF.getSubtarget().getRegisterInfo();
  MachineFrameInfo &MFI = MF.getFrameInfo();
  unsigned Size = TRI.getSpillSize(RC);
  Align Alignment = TRI.getSpillAlign(RC);
  EHSpillSlot[0] = MFI.CreateStackObject(Size, Alignment, true);
  EHSpillSlot[1] = MFI.CreateStackObject(Size, Alignment, true);
  EHSpillSlotSet = true;
  return EHSpillSlot;
}

