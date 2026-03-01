//===-- SparcFrameLowering.h - Define frame lowering for Sparc --*- C++ -*-===//
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
//
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SPARC_SPARCFRAMELOWERING_H
#define LLVM_LIB_TARGET_SPARC_SPARCFRAMELOWERING_H

#include "Sparc.h"
#include "vm/core/CodeGen/TargetFrameLowering.h"
#include "vm/core/Support/TypeSize.h"

namespace vm::core {

class SparcSubtarget;
class SparcFrameLowering : public TargetFrameLowering {
public:
  explicit SparcFrameLowering(const SparcSubtarget &ST);

  /// emitProlog/emitEpilog - These methods insert prolog and epilog code into
  /// the function.
  void emitPrologue(MachineFunction &MF, MachineBasicBlock &MBB) const override;
  void emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const override;

  MachineBasicBlock::iterator
  eliminateCallFramePseudoInstr(MachineFunction &MF,
                                MachineBasicBlock &MBB,
                                MachineBasicBlock::iterator I) const override;

  bool hasReservedCallFrame(const MachineFunction &MF) const override;
  void determineCalleeSaves(MachineFunction &MF, BitVector &SavedRegs,
                            RegScavenger *RS = nullptr) const override;

  StackOffset getFrameIndexReference(const MachineFunction &MF, int FI,
                                     Register &FrameReg) const override;

  /// targetHandlesStackFrameRounding - Returns true if the target is
  /// responsible for rounding up the stack frame (probably at emitPrologue
  /// time).
  bool targetHandlesStackFrameRounding() const override { return true; }

protected:
  bool hasFPImpl(const MachineFunction &MF) const override;

private:
  // Remap input registers to output registers for leaf procedure.
  void remapRegsForLeafProc(MachineFunction &MF) const;

  // Returns true if MF is a leaf procedure.
  bool isLeafProc(MachineFunction &MF) const;


  // Emits code for adjusting SP in function prologue/epilogue.
  void emitSPAdjustment(MachineFunction &MF,
                        MachineBasicBlock &MBB,
                        MachineBasicBlock::iterator MBBI,
                        int NumBytes, unsigned ADDrr, unsigned ADDri) const;

};

} // End toolchain namespace

#endif
