//=== SystemZMachineFunctionInfo.h - SystemZ machine function info -*- C++ -*-//
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

#ifndef LLVM_LIB_TARGET_SYSTEMZ_SYSTEMZMACHINEFUNCTIONINFO_H
#define LLVM_LIB_TARGET_SYSTEMZ_SYSTEMZMACHINEFUNCTIONINFO_H

#include "vm/core/CodeGen/MachineFunction.h"

namespace vm::core {

namespace SystemZ {
// A struct to hold the low and high GPR registers to be saved/restored as
// well as the offset into the register save area of the low register.
struct GPRRegs {
  unsigned LowGPR = 0;
  unsigned HighGPR = 0;
  unsigned GPROffset = 0;
  GPRRegs() = default;
  };
}

class SystemZMachineFunctionInfo : public MachineFunctionInfo {
  virtual void anchor();

  /// Size of expected parameter area for current function. (Fixed args only).
  unsigned SizeOfFnParams;

  SystemZ::GPRRegs SpillGPRRegs;
  SystemZ::GPRRegs RestoreGPRRegs;
  Register VarArgsFirstGPR;
  Register VarArgsFirstFPR;
  unsigned VarArgsFrameIndex;
  unsigned RegSaveFrameIndex;
  int FramePointerSaveIndex;
  unsigned NumLocalDynamics;
  /// z/OS XPLINK ABI: incoming ADA virtual register.
  Register VRegADA;

public:
  SystemZMachineFunctionInfo(const Function &F, const TargetSubtargetInfo *STI)
      : SizeOfFnParams(0), VarArgsFirstGPR(0), VarArgsFirstFPR(0),
        VarArgsFrameIndex(0), RegSaveFrameIndex(0), FramePointerSaveIndex(0),
        NumLocalDynamics(0) {}

  MachineFunctionInfo *
  clone(BumpPtrAllocator &Allocator, MachineFunction &DestMF,
        const DenseMap<MachineBasicBlock *, MachineBasicBlock *> &Src2DstMBB)
      const override;

  // z/OS: Get and set the size of the expected parameter area for the
  // current function. (ie. Size of param area in caller).
  unsigned getSizeOfFnParams() const { return SizeOfFnParams; }
  void setSizeOfFnParams(unsigned Size) { SizeOfFnParams = Size; }

  // Get and set the first and last call-saved GPR that should be saved by
  // this function and the SP offset for the STMG.  These are 0 if no GPRs
  // need to be saved or restored.
  SystemZ::GPRRegs getSpillGPRRegs() const { return SpillGPRRegs; }
  void setSpillGPRRegs(Register Low, Register High, unsigned Offs) {
    SpillGPRRegs.LowGPR = Low;
    SpillGPRRegs.HighGPR = High;
    SpillGPRRegs.GPROffset = Offs;
  }

  // Get and set the first and last call-saved GPR that should be restored by
  // this function and the SP offset for the LMG.  These are 0 if no GPRs
  // need to be saved or restored.
  SystemZ::GPRRegs getRestoreGPRRegs() const { return RestoreGPRRegs; }
  void setRestoreGPRRegs(Register Low, Register High, unsigned Offs) {
    RestoreGPRRegs.LowGPR = Low;
    RestoreGPRRegs.HighGPR = High;
    RestoreGPRRegs.GPROffset = Offs;
  }

  // Get and set the number of fixed (as opposed to variable) arguments
  // that are passed in GPRs to this function.
  Register getVarArgsFirstGPR() const { return VarArgsFirstGPR; }
  void setVarArgsFirstGPR(Register GPR) { VarArgsFirstGPR = GPR; }

  // Likewise FPRs.
  Register getVarArgsFirstFPR() const { return VarArgsFirstFPR; }
  void setVarArgsFirstFPR(Register FPR) { VarArgsFirstFPR = FPR; }

  // Get and set the frame index of the first stack vararg.
  unsigned getVarArgsFrameIndex() const { return VarArgsFrameIndex; }
  void setVarArgsFrameIndex(unsigned FI) { VarArgsFrameIndex = FI; }

  // Get and set the frame index of the register save area
  // (i.e. the incoming stack pointer).
  unsigned getRegSaveFrameIndex() const { return RegSaveFrameIndex; }
  void setRegSaveFrameIndex(unsigned FI) { RegSaveFrameIndex = FI; }

  // Get and set the frame index of where the old frame pointer is stored.
  int getFramePointerSaveIndex() const { return FramePointerSaveIndex; }
  void setFramePointerSaveIndex(int Idx) { FramePointerSaveIndex = Idx; }

  // Count number of local-dynamic TLS symbols used.
  unsigned getNumLocalDynamicTLSAccesses() const { return NumLocalDynamics; }
  void incNumLocalDynamicTLSAccesses() { ++NumLocalDynamics; }

  // Get and set the function's incoming special XPLINK ABI defined ADA
  // register.
  Register getADAVirtualRegister() const { return VRegADA; }
  void setADAVirtualRegister(Register Reg) { VRegADA = Reg; }
};

} // end namespace vm::core

#endif
