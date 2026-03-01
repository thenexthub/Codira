//===-- AVRMachineFuctionInfo.h - AVR machine function info -----*- C++ -*-===//
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
// This file declares AVR-specific per-machine-function information.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_AVR_MACHINE_FUNCTION_INFO_H
#define LLVM_AVR_MACHINE_FUNCTION_INFO_H

#include "vm/core/CodeGen/MachineFunction.h"

namespace vm::core {

/// Contains AVR-specific information for each MachineFunction.
class AVRMachineFunctionInfo : public MachineFunctionInfo {
  /// Indicates if a register has been spilled by the register
  /// allocator.
  bool HasSpills;

  /// Indicates if there are any fixed size allocas present.
  /// Note that if there are only variable sized allocas this is set to false.
  bool HasAllocas;

  /// Indicates if arguments passed using the stack are being
  /// used inside the function.
  bool HasStackArgs;

  /// Whether or not the function is an interrupt handler.
  bool IsInterruptHandler;

  /// Whether or not the function is an non-blocking interrupt handler.
  bool IsSignalHandler;

  /// Size of the callee-saved register portion of the
  /// stack frame in bytes.
  unsigned CalleeSavedFrameSize;

  /// FrameIndex for start of varargs area.
  int VarArgsFrameIndex;

public:
  AVRMachineFunctionInfo(const Function &F, const TargetSubtargetInfo *STI)
      : HasSpills(false), HasAllocas(false), HasStackArgs(false),
        CalleeSavedFrameSize(0), VarArgsFrameIndex(0) {
    CallingConv::ID CallConv = F.getCallingConv();

    this->IsInterruptHandler =
        CallConv == CallingConv::AVR_INTR || F.hasFnAttribute("interrupt");
    this->IsSignalHandler =
        CallConv == CallingConv::AVR_SIGNAL || F.hasFnAttribute("signal");
  }

  MachineFunctionInfo *
  clone(BumpPtrAllocator &Allocator, MachineFunction &DestMF,
        const DenseMap<MachineBasicBlock *, MachineBasicBlock *> &Src2DstMBB)
      const override {
    return DestMF.cloneInfo<AVRMachineFunctionInfo>(*this);
  }

  bool getHasSpills() const { return HasSpills; }
  void setHasSpills(bool B) { HasSpills = B; }

  bool getHasAllocas() const { return HasAllocas; }
  void setHasAllocas(bool B) { HasAllocas = B; }

  bool getHasStackArgs() const { return HasStackArgs; }
  void setHasStackArgs(bool B) { HasStackArgs = B; }

  /// Checks if the function is some form of interrupt service routine.
  bool isInterruptOrSignalHandler() const {
    return isInterruptHandler() || isSignalHandler();
  }

  bool isInterruptHandler() const { return IsInterruptHandler; }
  bool isSignalHandler() const { return IsSignalHandler; }

  unsigned getCalleeSavedFrameSize() const { return CalleeSavedFrameSize; }
  void setCalleeSavedFrameSize(unsigned Bytes) { CalleeSavedFrameSize = Bytes; }

  int getVarArgsFrameIndex() const { return VarArgsFrameIndex; }
  void setVarArgsFrameIndex(int Idx) { VarArgsFrameIndex = Idx; }
};

} // namespace vm::core

#endif // LLVM_AVR_MACHINE_FUNCTION_INFO_H
