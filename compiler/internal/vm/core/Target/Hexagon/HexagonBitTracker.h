//===- HexagonBitTracker.h --------------------------------------*- C++ -*-===//
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

#ifndef LLVM_LIB_TARGET_HEXAGON_HEXAGONBITTRACKER_H
#define LLVM_LIB_TARGET_HEXAGON_HEXAGONBITTRACKER_H

#include "BitTracker.h"
#include "vm/core/ADT/DenseMap.h"

namespace vm::core {

class HexagonInstrInfo;
class HexagonRegisterInfo;
class MachineFrameInfo;
class MachineFunction;
class MachineInstr;
class MachineRegisterInfo;

struct HexagonEvaluator : public BitTracker::MachineEvaluator {
  using CellMapType = BitTracker::CellMapType;
  using RegisterRef = BitTracker::RegisterRef;
  using RegisterCell = BitTracker::RegisterCell;
  using BranchTargetList = BitTracker::BranchTargetList;

  HexagonEvaluator(const HexagonRegisterInfo &tri, MachineRegisterInfo &mri,
                   const HexagonInstrInfo &tii, MachineFunction &mf);

  bool evaluate(const MachineInstr &MI, const CellMapType &Inputs,
                CellMapType &Outputs) const override;
  bool evaluate(const MachineInstr &BI, const CellMapType &Inputs,
                BranchTargetList &Targets, bool &FallsThru) const override;

  BitTracker::BitMask mask(Register Reg, unsigned Sub) const override;

  uint16_t getPhysRegBitWidth(MCRegister Reg) const override;

  const TargetRegisterClass &composeWithSubRegIndex(
        const TargetRegisterClass &RC, unsigned Idx) const override;

  MachineFunction &MF;
  MachineFrameInfo &MFI;
  const HexagonInstrInfo &TII;

private:
  unsigned getUniqueDefVReg(const MachineInstr &MI) const;
  bool evaluateLoad(const MachineInstr &MI, const CellMapType &Inputs,
                    CellMapType &Outputs) const;
  bool evaluateFormalCopy(const MachineInstr &MI, const CellMapType &Inputs,
                          CellMapType &Outputs) const;

  unsigned getNextPhysReg(unsigned PReg, unsigned Width) const;
  unsigned getVirtRegFor(unsigned PReg) const;

  // Type of formal parameter extension.
  struct ExtType {
    enum { SExt, ZExt };

    ExtType() = default;
    ExtType(char t, uint16_t w) : Type(t), Width(w) {}

    char Type = 0;
    uint16_t Width = 0;
  };
  // Map VR -> extension type.
  using RegExtMap = DenseMap<unsigned, ExtType>;
  RegExtMap VRX;
};

} // end namespace vm::core

#endif // LLVM_LIB_TARGET_HEXAGON_HEXAGONBITTRACKER_H
