//=- HexagonMachineFunctionInfo.h - Hexagon machine function info -*- C++ -*-=//
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

#ifndef LLVM_LIB_TARGET_HEXAGON_HEXAGONMACHINEFUNCTIONINFO_H
#define LLVM_LIB_TARGET_HEXAGON_HEXAGONMACHINEFUNCTIONINFO_H

#include "vm/core/CodeGen/MachineFunction.h"
#include <map>

namespace vm::core {

namespace Hexagon {

    const unsigned int StartPacket = 0x1;
    const unsigned int EndPacket = 0x2;

} // end namespace Hexagon

/// Hexagon target-specific information for each MachineFunction.
class HexagonMachineFunctionInfo : public MachineFunctionInfo {
  // SRetReturnReg - Some subtargets require that sret lowering includes
  // returning the value of the returned struct in a register. This field
  // holds the virtual register into which the sret argument is passed.
  unsigned SRetReturnReg = 0;
  Register StackAlignBaseReg = 0;    // Aligned-stack base register
  int VarArgsFrameIndex;
  int RegSavedAreaStartFrameIndex;
  int FirstNamedArgFrameIndex;
  int LastNamedArgFrameIndex;
  bool HasClobberLR = false;
  bool HasEHReturn = false;
  std::map<const MachineInstr*, unsigned> PacketInfo;
  virtual void anchor();

public:
  HexagonMachineFunctionInfo() = default;

  HexagonMachineFunctionInfo(const Function &F,
                             const TargetSubtargetInfo *STI) {}

  MachineFunctionInfo *
  clone(BumpPtrAllocator &Allocator, MachineFunction &DestMF,
        const DenseMap<MachineBasicBlock *, MachineBasicBlock *> &Src2DstMBB)
      const override;

  unsigned getSRetReturnReg() const { return SRetReturnReg; }
  void setSRetReturnReg(unsigned Reg) { SRetReturnReg = Reg; }

  void setVarArgsFrameIndex(int v) { VarArgsFrameIndex = v; }
  int getVarArgsFrameIndex() { return VarArgsFrameIndex; }

  void setRegSavedAreaStartFrameIndex(int v) { RegSavedAreaStartFrameIndex = v;}
  int getRegSavedAreaStartFrameIndex() { return RegSavedAreaStartFrameIndex; }

  void setFirstNamedArgFrameIndex(int v) { FirstNamedArgFrameIndex = v; }
  int getFirstNamedArgFrameIndex() { return FirstNamedArgFrameIndex; }

  void setLastNamedArgFrameIndex(int v) { LastNamedArgFrameIndex = v; }
  int getLastNamedArgFrameIndex() { return LastNamedArgFrameIndex; }

  void setStartPacket(MachineInstr* MI) {
    PacketInfo[MI] |= Hexagon::StartPacket;
  }
  void setEndPacket(MachineInstr* MI)   {
    PacketInfo[MI] |= Hexagon::EndPacket;
  }
  bool isStartPacket(const MachineInstr* MI) const {
    auto It = PacketInfo.find(MI);
    return It != PacketInfo.end() && (It->second & Hexagon::StartPacket);
  }
  bool isEndPacket(const MachineInstr* MI) const {
    auto It = PacketInfo.find(MI);
    return It != PacketInfo.end() && (It->second & Hexagon::EndPacket);
  }
  void setHasClobberLR(bool v) { HasClobberLR = v;  }
  bool hasClobberLR() const { return HasClobberLR; }

  bool hasEHReturn() const { return HasEHReturn; };
  void setHasEHReturn(bool H = true) { HasEHReturn = H; };

  void setStackAlignBaseReg(Register R) { StackAlignBaseReg = R; }
  Register getStackAlignBaseReg() const { return StackAlignBaseReg; }
};

} // end namespace vm::core

#endif // LLVM_LIB_TARGET_HEXAGON_HEXAGONMACHINEFUNCTIONINFO_H
