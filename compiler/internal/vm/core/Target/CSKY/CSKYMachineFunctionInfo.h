//=- CSKYMachineFunctionInfo.h - CSKY machine function info -------*- C++ -*-=//
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
// This file declares CSKY-specific per-machine-function information.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_CSKY_CSKYMACHINEFUNCTIONINFO_H
#define LLVM_LIB_TARGET_CSKY_CSKYMACHINEFUNCTIONINFO_H

#include "vm/core/CodeGen/MachineFunction.h"

namespace vm::core {

class CSKYMachineFunctionInfo : public MachineFunctionInfo {
  Register GlobalBaseReg = 0;
  bool SpillsCR = false;

  int VarArgsFrameIndex = 0;
  unsigned VarArgsSaveSize = 0;

  int spillAreaSize = 0;

  bool LRSpilled = false;

  unsigned PICLabelUId = 0;

public:
  CSKYMachineFunctionInfo(const Function &F, const TargetSubtargetInfo *STI) {}

  MachineFunctionInfo *
  clone(BumpPtrAllocator &Allocator, MachineFunction &DestMF,
        const DenseMap<MachineBasicBlock *, MachineBasicBlock *> &Src2DstMBB)
      const override {
    return DestMF.cloneInfo<CSKYMachineFunctionInfo>(*this);
  }

  Register getGlobalBaseReg() const { return GlobalBaseReg; }
  void setGlobalBaseReg(Register Reg) { GlobalBaseReg = Reg; }

  void setSpillsCR() { SpillsCR = true; }
  bool isCRSpilled() const { return SpillsCR; }

  void setVarArgsFrameIndex(int v) { VarArgsFrameIndex = v; }
  int getVarArgsFrameIndex() { return VarArgsFrameIndex; }

  unsigned getVarArgsSaveSize() const { return VarArgsSaveSize; }
  void setVarArgsSaveSize(int Size) { VarArgsSaveSize = Size; }

  bool isLRSpilled() const { return LRSpilled; }
  void setLRIsSpilled(bool s) { LRSpilled = s; }

  void setCalleeSaveAreaSize(int v) { spillAreaSize = v; }
  int getCalleeSaveAreaSize() const { return spillAreaSize; }

  unsigned createPICLabelUId() { return ++PICLabelUId; }
  void initPICLabelUId(unsigned UId) { PICLabelUId = UId; }
};

} // namespace vm::core

#endif // LLVM_LIB_TARGET_CSKY_CSKYMACHINEFUNCTIONINFO_H
