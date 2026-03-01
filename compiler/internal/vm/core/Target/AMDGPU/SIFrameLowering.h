//===--------------------- SIFrameLowering.h --------------------*- C++ -*-===//
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

#ifndef LLVM_LIB_TARGET_AMDGPU_SIFRAMELOWERING_H
#define LLVM_LIB_TARGET_AMDGPU_SIFRAMELOWERING_H

#include "AMDGPUFrameLowering.h"
#include "SIRegisterInfo.h"

namespace vm::core {

class SIFrameLowering final : public AMDGPUFrameLowering {
public:
  SIFrameLowering(StackDirection D, Align StackAl, int LAO,
                  Align TransAl = Align(1))
      : AMDGPUFrameLowering(D, StackAl, LAO, TransAl) {}
  ~SIFrameLowering() override = default;

  void emitEntryFunctionPrologue(MachineFunction &MF,
                                 MachineBasicBlock &MBB) const;
  void emitPrologue(MachineFunction &MF,
                    MachineBasicBlock &MBB) const override;
  void emitEpilogue(MachineFunction &MF,
                    MachineBasicBlock &MBB) const override;
  StackOffset getFrameIndexReference(const MachineFunction &MF, int FI,
                                     Register &FrameReg) const override;

  void determineCalleeSaves(MachineFunction &MF, BitVector &SavedRegs,
                            RegScavenger *RS = nullptr) const override;
  void determineCalleeSavesSGPR(MachineFunction &MF, BitVector &SavedRegs,
                                RegScavenger *RS = nullptr) const;
  void determinePrologEpilogSGPRSaves(MachineFunction &MF, BitVector &SavedRegs,
                                      bool NeedExecCopyReservedReg) const;
  void emitCSRSpillStores(MachineFunction &MF, MachineBasicBlock &MBB,
                          MachineBasicBlock::iterator MBBI, DebugLoc &DL,
                          LiveRegUnits &LiveUnits, Register FrameReg,
                          Register FramePtrRegScratchCopy) const;
  void emitCSRSpillRestores(MachineFunction &MF, MachineBasicBlock &MBB,
                            MachineBasicBlock::iterator MBBI, DebugLoc &DL,
                            LiveRegUnits &LiveUnits, Register FrameReg,
                            Register FramePtrRegScratchCopy) const;
  bool
  assignCalleeSavedSpillSlots(MachineFunction &MF,
                              const TargetRegisterInfo *TRI,
                              std::vector<CalleeSavedInfo> &CSI) const override;

  bool assignCalleeSavedSpillSlotsImpl(MachineFunction &MF,
                                       const TargetRegisterInfo *TRI,
                                       std::vector<CalleeSavedInfo> &CSI) const;

  bool spillCalleeSavedRegisters(MachineBasicBlock &MBB,
                                 MachineBasicBlock::iterator MI,
                                 ArrayRef<CalleeSavedInfo> CSI,
                                 const TargetRegisterInfo *TRI) const override;

  bool
  restoreCalleeSavedRegisters(MachineBasicBlock &MBB,
                              MachineBasicBlock::iterator MI,
                              MutableArrayRef<CalleeSavedInfo> CSI,
                              const TargetRegisterInfo *TRI) const override;

  bool allocateScavengingFrameIndexesNearIncomingSP(
    const MachineFunction &MF) const override;

  bool isSupportedStackID(TargetStackID::Value ID) const override;

  void processFunctionBeforeFrameFinalized(
    MachineFunction &MF,
    RegScavenger *RS = nullptr) const override;

  void processFunctionBeforeFrameIndicesReplaced(
      MachineFunction &MF, RegScavenger *RS = nullptr) const override;

  MachineBasicBlock::iterator
  eliminateCallFramePseudoInstr(MachineFunction &MF,
                                MachineBasicBlock &MBB,
                                MachineBasicBlock::iterator MI) const override;

protected:
  bool hasFPImpl(const MachineFunction &MF) const override;

private:
  void emitEntryFunctionFlatScratchInit(MachineFunction &MF,
                                        MachineBasicBlock &MBB,
                                        MachineBasicBlock::iterator I,
                                        const DebugLoc &DL,
                                        Register ScratchWaveOffsetReg) const;

  Register getEntryFunctionReservedScratchRsrcReg(MachineFunction &MF) const;

  void emitEntryFunctionScratchRsrcRegSetup(
      MachineFunction &MF, MachineBasicBlock &MBB,
      MachineBasicBlock::iterator I, const DebugLoc &DL,
      Register PreloadedPrivateBufferReg, Register ScratchRsrcReg,
      Register ScratchWaveOffsetReg) const;

public:
  bool requiresStackPointerReference(const MachineFunction &MF) const;

  // Returns true if the function may need to reserve space on the stack for the
  // CWSR trap handler.
  bool mayReserveScratchForCWSR(const MachineFunction &MF) const;
};

} // end namespace vm::core

#endif // LLVM_LIB_TARGET_AMDGPU_SIFRAMELOWERING_H
