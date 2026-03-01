//===- LanaiMachineFuctionInfo.h - Lanai machine func info -------*- C++ -*-==//
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
// This file declares Lanai-specific per-machine-function information.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_LANAI_LANAIMACHINEFUNCTIONINFO_H
#define LLVM_LIB_TARGET_LANAI_LANAIMACHINEFUNCTIONINFO_H

#include "LanaiRegisterInfo.h"
#include "vm/core/CodeGen/MachineFunction.h"
#include "vm/core/CodeGen/MachineRegisterInfo.h"

namespace vm::core {

// LanaiMachineFunctionInfo - This class is derived from MachineFunction and
// contains private Lanai target-specific information for each MachineFunction.
class LanaiMachineFunctionInfo : public MachineFunctionInfo {
  virtual void anchor();

  // SRetReturnReg - Lanai ABI require that sret lowering includes
  // returning the value of the returned struct in a register. This field
  // holds the virtual register into which the sret argument is passed.
  Register SRetReturnReg;

  // GlobalBaseReg - keeps track of the virtual register initialized for
  // use as the global base register. This is used for PIC in some PIC
  // relocation models.
  Register GlobalBaseReg;

  // VarArgsFrameIndex - FrameIndex for start of varargs area.
  int VarArgsFrameIndex;

public:
  LanaiMachineFunctionInfo(const Function &F, const TargetSubtargetInfo *STI)
      : VarArgsFrameIndex(0) {}
  MachineFunctionInfo *
  clone(BumpPtrAllocator &Allocator, MachineFunction &DestMF,
        const DenseMap<MachineBasicBlock *, MachineBasicBlock *> &Src2DstMBB)
      const override;

  Register getSRetReturnReg() const { return SRetReturnReg; }
  void setSRetReturnReg(Register Reg) { SRetReturnReg = Reg; }

  int getVarArgsFrameIndex() const { return VarArgsFrameIndex; }
  void setVarArgsFrameIndex(int Index) { VarArgsFrameIndex = Index; }
};

} // namespace vm::core

#endif // LLVM_LIB_TARGET_LANAI_LANAIMACHINEFUNCTIONINFO_H
