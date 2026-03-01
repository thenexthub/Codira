//=- LoongArchMachineFunctionInfo.h - LoongArch machine function info -----===//
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
// This file declares LoongArch-specific per-machine-function information.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_LOONGARCH_LOONGARCHMACHINEFUNCTIONINFO_H
#define LLVM_LIB_TARGET_LOONGARCH_LOONGARCHMACHINEFUNCTIONINFO_H

#include "LoongArchSubtarget.h"
#include "vm/core/CodeGen/MachineFrameInfo.h"
#include "vm/core/CodeGen/MachineFunction.h"

namespace vm::core {

/// LoongArchMachineFunctionInfo - This class is derived from
/// MachineFunctionInfo and contains private LoongArch-specific information for
/// each MachineFunction.
class LoongArchMachineFunctionInfo : public MachineFunctionInfo {
private:
  /// FrameIndex for start of varargs area
  int VarArgsFrameIndex = 0;
  /// Size of the save area used for varargs
  int VarArgsSaveSize = 0;

  /// Size of stack frame to save callee saved registers
  unsigned CalleeSavedStackSize = 0;

  /// Amount of bytes on stack consumed by the arguments being passed on
  /// the stack
  unsigned ArgumentStackSize = 0;

  /// FrameIndex of the spill slot when there is no scavenged register in
  /// insertIndirectBranch.
  int BranchRelaxationSpillFrameIndex = -1;

  /// Incoming ByVal arguments
  SmallVector<SDValue, 8> IncomingByValArgs;

  /// Registers that have been sign extended from i32.
  SmallVector<Register, 8> SExt32Registers;

  /// Pairs of `jr` instructions and corresponding JTI operands, used for the
  /// `annotate-tablejump` option.
  SmallVector<std::pair<MachineInstr *, int>, 4> JumpInfos;

public:
  LoongArchMachineFunctionInfo(const Function &F,
                               const TargetSubtargetInfo *STI) {}

  MachineFunctionInfo *
  clone(BumpPtrAllocator &Allocator, MachineFunction &DestMF,
        const DenseMap<MachineBasicBlock *, MachineBasicBlock *> &Src2DstMBB)
      const override {
    return DestMF.cloneInfo<LoongArchMachineFunctionInfo>(*this);
  }

  int getVarArgsFrameIndex() const { return VarArgsFrameIndex; }
  void setVarArgsFrameIndex(int Index) { VarArgsFrameIndex = Index; }

  unsigned getVarArgsSaveSize() const { return VarArgsSaveSize; }
  void setVarArgsSaveSize(int Size) { VarArgsSaveSize = Size; }

  unsigned getCalleeSavedStackSize() const { return CalleeSavedStackSize; }
  void setCalleeSavedStackSize(unsigned Size) { CalleeSavedStackSize = Size; }

  unsigned getArgumentStackSize() const { return ArgumentStackSize; }
  void setArgumentStackSize(unsigned size) { ArgumentStackSize = size; }

  int getBranchRelaxationSpillFrameIndex() {
    return BranchRelaxationSpillFrameIndex;
  }
  void setBranchRelaxationSpillFrameIndex(int Index) {
    BranchRelaxationSpillFrameIndex = Index;
  }

  void addIncomingByValArgs(SDValue Val) { IncomingByValArgs.push_back(Val); }
  SDValue getIncomingByValArgs(int Idx) { return IncomingByValArgs[Idx]; }
  unsigned getIncomingByValArgsSize() const { return IncomingByValArgs.size(); }

  void addSExt32Register(Register Reg) { SExt32Registers.push_back(Reg); }

  bool isSExt32Register(Register Reg) const {
    return is_contained(SExt32Registers, Reg);
  }

  void setJumpInfo(MachineInstr *JrMI, int JTIIdx) {
    JumpInfos.push_back(std::make_pair(JrMI, JTIIdx));
  }
  unsigned getJumpInfoSize() { return JumpInfos.size(); }
  MachineInstr *getJumpInfoJrMI(unsigned Idx) { return JumpInfos[Idx].first; }
  int getJumpInfoJTIIndex(unsigned Idx) { return JumpInfos[Idx].second; }
};

} // end namespace vm::core

#endif // LLVM_LIB_TARGET_LOONGARCH_LOONGARCHMACHINEFUNCTIONINFO_H
