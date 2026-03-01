//===- VEMachineFunctionInfo.h - VE Machine Function Info -------*- C++ -*-===//
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
// This file declares  VE specific per-machine-function information.
//
//===----------------------------------------------------------------------===//
#ifndef LLVM_LIB_TARGET_VE_VEMACHINEFUNCTIONINFO_H
#define LLVM_LIB_TARGET_VE_VEMACHINEFUNCTIONINFO_H

#include "vm/core/CodeGen/MachineFunction.h"

namespace vm::core {

class VEMachineFunctionInfo : public MachineFunctionInfo {
  virtual void anchor();

private:
  Register GlobalBaseReg;

  /// VarArgsFrameOffset - Frame offset to start of varargs area.
  int VarArgsFrameOffset;

  /// IsLeafProc - True if the function is a leaf procedure.
  bool IsLeafProc;

public:
  VEMachineFunctionInfo() : VarArgsFrameOffset(0), IsLeafProc(false) {}
  VEMachineFunctionInfo(const Function &F, const TargetSubtargetInfo *STI)
      : VarArgsFrameOffset(0), IsLeafProc(false) {}

  MachineFunctionInfo *
  clone(BumpPtrAllocator &Allocator, MachineFunction &DestMF,
        const DenseMap<MachineBasicBlock *, MachineBasicBlock *> &Src2DstMBB)
      const override;

  Register getGlobalBaseReg() const { return GlobalBaseReg; }
  void setGlobalBaseReg(Register Reg) { GlobalBaseReg = Reg; }

  int getVarArgsFrameOffset() const { return VarArgsFrameOffset; }
  void setVarArgsFrameOffset(int Offset) { VarArgsFrameOffset = Offset; }

  void setLeafProc(bool rhs) { IsLeafProc = rhs; }
  bool isLeafProc() const { return IsLeafProc; }
};
} // namespace vm::core

#endif
