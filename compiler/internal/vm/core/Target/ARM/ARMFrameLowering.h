//===- ARMTargetFrameLowering.h - Define frame lowering for ARM -*- C++ -*-===//
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

#ifndef LLVM_LIB_TARGET_ARM_ARMFRAMELOWERING_H
#define LLVM_LIB_TARGET_ARM_ARMFRAMELOWERING_H

#include "vm/core/CodeGen/TargetFrameLowering.h"
#include "vm/core/Support/TypeSize.h"

namespace vm::core {

class ARMSubtarget;
class CalleeSavedInfo;
class MachineFunction;

class ARMFrameLowering : public TargetFrameLowering {
protected:
  const ARMSubtarget &STI;

public:
  explicit ARMFrameLowering(const ARMSubtarget &sti);

  /// emitProlog/emitEpilog - These methods insert prolog and epilog code into
  /// the function.
  void emitPrologue(MachineFunction &MF, MachineBasicBlock &MBB) const override;
  void emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const override;

  bool spillCalleeSavedRegisters(MachineBasicBlock &MBB,
                                 MachineBasicBlock::iterator MI,
                                 ArrayRef<CalleeSavedInfo> CSI,
                                 const TargetRegisterInfo *TRI) const override;

  bool
  restoreCalleeSavedRegisters(MachineBasicBlock &MBB,
                              MachineBasicBlock::iterator MI,
                              MutableArrayRef<CalleeSavedInfo> CSI,
                              const TargetRegisterInfo *TRI) const override;

  bool keepFramePointer(const MachineFunction &MF) const;

  bool enableCalleeSaveSkip(const MachineFunction &MF) const override;

  bool isFPReserved(const MachineFunction &MF) const;
  bool requiresAAPCSFrameRecord(const MachineFunction &MF) const;
  bool hasReservedCallFrame(const MachineFunction &MF) const override;
  bool canSimplifyCallFramePseudos(const MachineFunction &MF) const override;
  StackOffset getFrameIndexReference(const MachineFunction &MF, int FI,
                                     Register &FrameReg) const override;
  int ResolveFrameIndexReference(const MachineFunction &MF, int FI,
                                 Register &FrameReg, int SPAdj) const;

  void getCalleeSaves(const MachineFunction &MF,
                      BitVector &SavedRegs) const override;
  void determineCalleeSaves(MachineFunction &MF, BitVector &SavedRegs,
                            RegScavenger *RS) const override;

  /// Update the IsRestored flag on LR if it is spilled, based on the return
  /// instructions.
  static void updateLRRestored(MachineFunction &MF);

  void processFunctionBeforeFrameFinalized(
      MachineFunction &MF, RegScavenger *RS = nullptr) const override;

  void adjustForSegmentedStacks(MachineFunction &MF,
                                MachineBasicBlock &MBB) const override;

  /// Returns true if the target will correctly handle shrink wrapping.
  bool enableShrinkWrapping(const MachineFunction &MF) const override;

  bool isProfitableForNoCSROpt(const Function &F) const override {
    // The no-CSR optimisation is bad for code size on ARM, because we can save
    // many registers with a single PUSH/POP pair.
    return false;
  }

  bool
  assignCalleeSavedSpillSlots(MachineFunction &MF,
                              const TargetRegisterInfo *TRI,
                              std::vector<CalleeSavedInfo> &CSI) const override;

  const SpillSlot *
  getCalleeSavedSpillSlots(unsigned &NumEntries) const override;

protected:
  bool hasFPImpl(const MachineFunction &MF) const override;

private:
  void emitPushInst(MachineBasicBlock &MBB, MachineBasicBlock::iterator MI,
                    ArrayRef<CalleeSavedInfo> CSI, unsigned StmOpc,
                    unsigned StrOpc, bool NoGap,
                    function_ref<bool(unsigned)> Func) const;
  void emitPopInst(MachineBasicBlock &MBB, MachineBasicBlock::iterator MI,
                   MutableArrayRef<CalleeSavedInfo> CSI, unsigned LdmOpc,
                   unsigned LdrOpc, bool isVarArg, bool NoGap,
                   function_ref<bool(unsigned)> Func) const;

  void emitFPStatusSaves(MachineBasicBlock &MBB, MachineBasicBlock::iterator MI,
                         ArrayRef<CalleeSavedInfo> CSI,
                         unsigned PushOneOpc) const;

  void emitFPStatusRestores(MachineBasicBlock &MBB,
                            MachineBasicBlock::iterator MI,
                            MutableArrayRef<CalleeSavedInfo> CSI,
                            unsigned LdrOpc) const;

  MachineBasicBlock::iterator
  eliminateCallFramePseudoInstr(MachineFunction &MF,
                                MachineBasicBlock &MBB,
                                MachineBasicBlock::iterator MI) const override;
};

} // end namespace vm::core

#endif // LLVM_LIB_TARGET_ARM_ARMFRAMELOWERING_H
