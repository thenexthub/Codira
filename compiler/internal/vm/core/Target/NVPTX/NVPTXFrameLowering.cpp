//=======- NVPTXFrameLowering.cpp - NVPTX Frame Information ---*- C++ -*-=====//
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
// This file contains the NVPTX implementation of TargetFrameLowering class.
//
//===----------------------------------------------------------------------===//

#include "NVPTXFrameLowering.h"
#include "NVPTX.h"
#include "NVPTXRegisterInfo.h"
#include "NVPTXSubtarget.h"
#include "NVPTXTargetMachine.h"
#include "vm/core/CodeGen/MachineFrameInfo.h"
#include "vm/core/CodeGen/MachineFunction.h"
#include "vm/core/CodeGen/MachineInstrBuilder.h"
#include "vm/core/CodeGen/MachineRegisterInfo.h"
#include "vm/core/CodeGen/TargetInstrInfo.h"

using namespace vm::core;

NVPTXFrameLowering::NVPTXFrameLowering()
    : TargetFrameLowering(TargetFrameLowering::StackGrowsUp, Align(8), 0) {}

bool NVPTXFrameLowering::hasFPImpl(const MachineFunction &MF) const {
  return true;
}

void NVPTXFrameLowering::emitPrologue(MachineFunction &MF,
                                      MachineBasicBlock &MBB) const {
  if (MF.getFrameInfo().hasStackObjects()) {
    assert(&MF.front() == &MBB && "Shrink-wrapping not yet supported");
    MachineBasicBlock::iterator MBBI = MBB.begin();
    MachineRegisterInfo &MR = MF.getRegInfo();

    const NVPTXRegisterInfo *NRI =
        MF.getSubtarget<NVPTXSubtarget>().getRegisterInfo();

    // This instruction really occurs before first instruction
    // in the BB, so giving it no debug location.
    DebugLoc dl = DebugLoc();

    // Emits
    //   mov %SPL, %depot;
    //   cvta.local %SP, %SPL;
    // for local address accesses in MF.
    bool Is64Bit =
        static_cast<const NVPTXTargetMachine &>(MF.getTarget()).is64Bit();
    unsigned CvtaLocalOpcode =
        (Is64Bit ? NVPTX::cvta_local_64 : NVPTX::cvta_local);
    unsigned MovDepotOpcode =
        (Is64Bit ? NVPTX::MOV_DEPOT_ADDR_64 : NVPTX::MOV_DEPOT_ADDR);
    if (!MR.use_empty(NRI->getFrameRegister(MF))) {
      // If %SP is not used, do not bother emitting "cvta.local %SP, %SPL".
      MBBI = BuildMI(MBB, MBBI, dl,
                     MF.getSubtarget().getInstrInfo()->get(CvtaLocalOpcode),
                     NRI->getFrameRegister(MF))
                 .addReg(NRI->getFrameLocalRegister(MF));
    }
    if (!MR.use_empty(NRI->getFrameLocalRegister(MF))) {
      BuildMI(MBB, MBBI, dl,
              MF.getSubtarget().getInstrInfo()->get(MovDepotOpcode),
              NRI->getFrameLocalRegister(MF))
          .addImm(MF.getFunctionNumber());
    }
  }
}

StackOffset
NVPTXFrameLowering::getFrameIndexReference(const MachineFunction &MF, int FI,
                                           Register &FrameReg) const {
  const MachineFrameInfo &MFI = MF.getFrameInfo();
  FrameReg = NVPTX::VRDepot;
  return StackOffset::getFixed(MFI.getObjectOffset(FI) -
                               getOffsetOfLocalArea());
}

void NVPTXFrameLowering::emitEpilogue(MachineFunction &MF,
                                      MachineBasicBlock &MBB) const {}

// This function eliminates ADJCALLSTACKDOWN,
// ADJCALLSTACKUP pseudo instructions
MachineBasicBlock::iterator NVPTXFrameLowering::eliminateCallFramePseudoInstr(
    MachineFunction &MF, MachineBasicBlock &MBB,
    MachineBasicBlock::iterator I) const {
  // Simply discard ADJCALLSTACKDOWN,
  // ADJCALLSTACKUP instructions.
  return MBB.erase(I);
}

TargetFrameLowering::DwarfFrameBase
NVPTXFrameLowering::getDwarfFrameBase(const MachineFunction &MF) const {
  DwarfFrameBase FrameBase;
  FrameBase.Kind = DwarfFrameBase::CFA;
  FrameBase.Location.Offset = 0;
  return FrameBase;
}
