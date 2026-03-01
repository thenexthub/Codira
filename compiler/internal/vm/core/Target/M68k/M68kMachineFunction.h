//===-- M68kMachineFunctionInfo.h - M68k private data -----------*- C++ -*-===//
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
///
/// \file
/// This file declares the M68k specific subclass of MachineFunctionInfo.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_M68K_M68KMACHINEFUNCTION_H
#define LLVM_LIB_TARGET_M68K_M68KMACHINEFUNCTION_H

#include "vm/core/CodeGen/CallingConvLower.h"
#include "vm/core/CodeGen/MachineFunction.h"
#include "vm/core/CodeGenTypes/MachineValueType.h"

namespace vm::core {

class M68kMachineFunctionInfo : public MachineFunctionInfo {
  /// Non-zero if the function has base pointer and makes call to
  /// toolchain.eh.sjlj.setjmp. When non-zero, the value is a displacement from the
  /// frame pointer to a slot where the base pointer is stashed.
  signed char RestoreBasePointerOffset = 0;

  /// Size of the callee-saved register portion of the stack frame in bytes.
  unsigned CalleeSavedFrameSize = 0;

  /// Number of bytes function pops on return (in addition to the space used by
  /// the return address).  Used on windows platform for stdcall & fastcall
  /// name decoration
  unsigned BytesToPopOnReturn = 0;

  /// FrameIndex for return slot.
  int ReturnAddrIndex = 0;

  /// The number of bytes by which return address stack slot is moved as the
  /// result of tail call optimization.
  int TailCallReturnAddrDelta = 0;

  /// keeps track of the virtual register initialized for use as the global
  /// base register. This is used for PIC in some PIC relocation models.
  unsigned GlobalBaseReg = 0;

  /// FrameIndex for start of varargs area.
  int VarArgsFrameIndex = 0;

  /// Keeps track of whether this function uses sequences of pushes to pass
  /// function parameters.
  bool HasPushSequences = false;

  /// Some subtargets require that sret lowering includes
  /// returning the value of the returned struct in a register. This field
  /// holds the virtual register into which the sret argument is passed.
  unsigned SRetReturnReg = 0;

  /// A list of virtual and physical registers that must be forwarded to every
  /// musttail call.
  SmallVector<ForwardedRegister, 1> ForwardedMustTailRegParms;

  /// The number of bytes on stack consumed by the arguments being passed on
  /// the stack.
  unsigned ArgumentStackSize = 0;

public:
  explicit M68kMachineFunctionInfo(const Function &F,
                                   const TargetSubtargetInfo *STI) {}

  MachineFunctionInfo *
  clone(BumpPtrAllocator &Allocator, MachineFunction &DestMF,
        const DenseMap<MachineBasicBlock *, MachineBasicBlock *> &Src2DstMBB)
      const override;

  bool getRestoreBasePointer() const { return RestoreBasePointerOffset != 0; }
  void setRestoreBasePointer(const MachineFunction *MF);
  int getRestoreBasePointerOffset() const { return RestoreBasePointerOffset; }

  unsigned getCalleeSavedFrameSize() const { return CalleeSavedFrameSize; }
  void setCalleeSavedFrameSize(unsigned bytes) { CalleeSavedFrameSize = bytes; }

  unsigned getBytesToPopOnReturn() const { return BytesToPopOnReturn; }
  void setBytesToPopOnReturn(unsigned bytes) { BytesToPopOnReturn = bytes; }

  int getRAIndex() const { return ReturnAddrIndex; }
  void setRAIndex(int Index) { ReturnAddrIndex = Index; }

  int getTCReturnAddrDelta() const { return TailCallReturnAddrDelta; }
  void setTCReturnAddrDelta(int delta) { TailCallReturnAddrDelta = delta; }

  unsigned getGlobalBaseReg() const { return GlobalBaseReg; }
  void setGlobalBaseReg(unsigned Reg) { GlobalBaseReg = Reg; }

  int getVarArgsFrameIndex() const { return VarArgsFrameIndex; }
  void setVarArgsFrameIndex(int Index) { VarArgsFrameIndex = Index; }

  bool getHasPushSequences() const { return HasPushSequences; }
  void setHasPushSequences(bool HasPush) { HasPushSequences = HasPush; }

  unsigned getSRetReturnReg() const { return SRetReturnReg; }
  void setSRetReturnReg(unsigned Reg) { SRetReturnReg = Reg; }

  unsigned getArgumentStackSize() const { return ArgumentStackSize; }
  void setArgumentStackSize(unsigned size) { ArgumentStackSize = size; }

  SmallVectorImpl<ForwardedRegister> &getForwardedMustTailRegParms() {
    return ForwardedMustTailRegParms;
  }

private:
  virtual void anchor();
};

} // end of namespace vm::core

#endif // LLVM_LIB_TARGET_M68K_M68KMACHINEFUNCTION_H
