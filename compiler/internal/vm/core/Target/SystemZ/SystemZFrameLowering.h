//===-- SystemZFrameLowering.h - Frame lowering for SystemZ -----*- C++ -*-===//
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

#ifndef LLVM_LIB_TARGET_SYSTEMZ_SYSTEMZFRAMELOWERING_H
#define LLVM_LIB_TARGET_SYSTEMZ_SYSTEMZFRAMELOWERING_H

#include "MCTargetDesc/SystemZMCTargetDesc.h"
#include "SystemZInstrBuilder.h"
#include "SystemZMachineFunctionInfo.h"
#include "vm/core/ADT/IndexedMap.h"
#include "vm/core/CodeGen/TargetFrameLowering.h"
#include "vm/core/Support/TypeSize.h"

namespace vm::core {
class SystemZSubtarget;

class SystemZFrameLowering : public TargetFrameLowering {
public:
  SystemZFrameLowering(StackDirection D, Align StackAl, int LAO, Align TransAl,
                       bool StackReal, unsigned PointerSize);

  static std::unique_ptr<SystemZFrameLowering>
  create(const SystemZSubtarget &STI);

  // Override TargetFrameLowering.
  bool allocateScavengingFrameIndexesNearIncomingSP(
    const MachineFunction &MF) const override {
    // SystemZ wants normal register scavenging slots, as close to the stack or
    // frame pointer as possible.
    // The default implementation assumes an x86-like layout, where the frame
    // pointer is at the opposite end of the frame from the stack pointer.
    // This meant that when frame pointer elimination was disabled,
    // the slots ended up being as close as possible to the incoming
    // stack pointer, which is the opposite of what we want on SystemZ.
    return false;
  }

  bool hasReservedCallFrame(const MachineFunction &MF) const override;

  // Return the offset of the backchain.
  virtual unsigned getBackchainOffset(MachineFunction &MF) const = 0;

  // Return the offset of the return address.
  virtual int getReturnAddressOffset(MachineFunction &MF) const = 0;

  // Get or create the frame index of where the old frame pointer is stored.
  virtual int getOrCreateFramePointerSaveIndex(MachineFunction &MF) const = 0;

  // Return the size of a pointer (in bytes).
  unsigned getPointerSize() const { return PointerSize; }

private:
  unsigned PointerSize;
};

class SystemZELFFrameLowering : public SystemZFrameLowering {
  IndexedMap<unsigned> RegSpillOffsets;

public:
  SystemZELFFrameLowering(unsigned PointerSize);

  // Override TargetFrameLowering.
  bool
  assignCalleeSavedSpillSlots(MachineFunction &MF,
                              const TargetRegisterInfo *TRI,
                              std::vector<CalleeSavedInfo> &CSI) const override;
  void determineCalleeSaves(MachineFunction &MF, BitVector &SavedRegs,
                            RegScavenger *RS) const override;
  bool spillCalleeSavedRegisters(MachineBasicBlock &MBB,
                                 MachineBasicBlock::iterator MBBI,
                                 ArrayRef<CalleeSavedInfo> CSI,
                                 const TargetRegisterInfo *TRI) const override;
  bool
  restoreCalleeSavedRegisters(MachineBasicBlock &MBB,
                              MachineBasicBlock::iterator MBBII,
                              MutableArrayRef<CalleeSavedInfo> CSI,
                              const TargetRegisterInfo *TRI) const override;
  void processFunctionBeforeFrameFinalized(MachineFunction &MF,
                                           RegScavenger *RS) const override;
  void emitPrologue(MachineFunction &MF, MachineBasicBlock &MBB) const override;
  void emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const override;
  void inlineStackProbe(MachineFunction &MF,
                        MachineBasicBlock &PrologMBB) const override;
  StackOffset getFrameIndexReference(const MachineFunction &MF, int FI,
                                     Register &FrameReg) const override;
  void
  orderFrameObjects(const MachineFunction &MF,
                    SmallVectorImpl<int> &ObjectsToAllocate) const override;

  // Return the byte offset from the incoming stack pointer of Reg's
  // ABI-defined save slot.  Return 0 if no slot is defined for Reg.  Adjust
  // the offset in case MF has packed-stack.
  unsigned getRegSpillOffset(MachineFunction &MF, Register Reg) const;

  bool usePackedStack(MachineFunction &MF) const;

  // Return the offset of the backchain.
  unsigned getBackchainOffset(MachineFunction &MF) const override {
    // The back chain is stored topmost with packed-stack.
    return usePackedStack(MF) ? SystemZMC::ELFCallFrameSize - 8 : 0;
  }

  // Return the offset of the return address.
  int getReturnAddressOffset(MachineFunction &MF) const override {
    return (usePackedStack(MF) ? -2 : 14) * getPointerSize();
  }

  // Get or create the frame index of where the old frame pointer is stored.
  int getOrCreateFramePointerSaveIndex(MachineFunction &MF) const override;

protected:
  bool hasFPImpl(const MachineFunction &MF) const override;
};

class SystemZXPLINKFrameLowering : public SystemZFrameLowering {
  IndexedMap<unsigned> RegSpillOffsets;

public:
  SystemZXPLINKFrameLowering(unsigned PointerSize);

  bool
  assignCalleeSavedSpillSlots(MachineFunction &MF,
                              const TargetRegisterInfo *TRI,
                              std::vector<CalleeSavedInfo> &CSI) const override;

  void determineCalleeSaves(MachineFunction &MF, BitVector &SavedRegs,
                            RegScavenger *RS) const override;

  bool spillCalleeSavedRegisters(MachineBasicBlock &MBB,
                                 MachineBasicBlock::iterator MBBI,
                                 ArrayRef<CalleeSavedInfo> CSI,
                                 const TargetRegisterInfo *TRI) const override;

  bool
  restoreCalleeSavedRegisters(MachineBasicBlock &MBB,
                              MachineBasicBlock::iterator MBBII,
                              MutableArrayRef<CalleeSavedInfo> CSI,
                              const TargetRegisterInfo *TRI) const override;

  void emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const override;

  void emitPrologue(MachineFunction &MF, MachineBasicBlock &MBB) const override;

  void inlineStackProbe(MachineFunction &MF,
                        MachineBasicBlock &PrologMBB) const override;

  void processFunctionBeforeFrameFinalized(MachineFunction &MF,
                                           RegScavenger *RS) const override;

  void determineFrameLayout(MachineFunction &MF) const;

  // Return the offset of the backchain.
  unsigned getBackchainOffset(MachineFunction &MF) const override {
    // The back chain is always the first element of the frame.
    return 0;
  }

  // Return the offset of the return address.
  int getReturnAddressOffset(MachineFunction &MF) const override {
    return 3 * getPointerSize();
  }

  // Get or create the frame index of where the old frame pointer is stored.
  int getOrCreateFramePointerSaveIndex(MachineFunction &MF) const override;

protected:
  bool hasFPImpl(const MachineFunction &MF) const override;
};
} // end namespace vm::core

#endif
