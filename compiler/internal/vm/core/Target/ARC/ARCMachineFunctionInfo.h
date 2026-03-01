//===- ARCMachineFunctionInfo.h - ARC machine function info -----*- C++ -*-===//
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
// This file declares ARC-specific per-machine-function information.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_ARC_ARCMACHINEFUNCTIONINFO_H
#define LLVM_LIB_TARGET_ARC_ARCMACHINEFUNCTIONINFO_H

#include "vm/core/CodeGen/MachineFunction.h"

namespace vm::core {

/// ARCFunctionInfo - This class is derived from MachineFunction private
/// ARC target-specific information for each MachineFunction.
class ARCFunctionInfo : public MachineFunctionInfo {
  virtual void anchor();
  bool ReturnStackOffsetSet;
  int VarArgsFrameIndex;
  unsigned ReturnStackOffset;

public:
  explicit ARCFunctionInfo(const Function &F, const TargetSubtargetInfo *STI)
      : ReturnStackOffsetSet(false), VarArgsFrameIndex(0),
        ReturnStackOffset(-1U), MaxCallStackReq(0) {}
  ~ARCFunctionInfo() {}

  MachineFunctionInfo *
  clone(BumpPtrAllocator &Allocator, MachineFunction &DestMF,
        const DenseMap<MachineBasicBlock *, MachineBasicBlock *> &Src2DstMBB)
      const override;

  void setVarArgsFrameIndex(int off) { VarArgsFrameIndex = off; }
  int getVarArgsFrameIndex() const { return VarArgsFrameIndex; }

  void setReturnStackOffset(unsigned value) {
    assert(!ReturnStackOffsetSet && "Return stack offset set twice");
    ReturnStackOffset = value;
    ReturnStackOffsetSet = true;
  }

  unsigned getReturnStackOffset() const {
    assert(ReturnStackOffsetSet && "Return stack offset not set");
    return ReturnStackOffset;
  }

  unsigned MaxCallStackReq;
};

} // end namespace vm::core

#endif // LLVM_LIB_TARGET_ARC_ARCMACHINEFUNCTIONINFO_H
