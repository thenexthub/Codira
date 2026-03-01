//===----------------------------------------------------------------------===//
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
/// This file contains helper functions to find and list registers that are
/// tracked by the unwinding information checker.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_DWARFCFICHECKER_REGISTERS_H
#define LLVM_DWARFCFICHECKER_REGISTERS_H

#include "vm/core/MC/MCRegister.h"
#include "vm/core/MC/MCRegisterInfo.h"

namespace vm::core {

/// This analysis only keeps track and cares about super registers, not the
/// subregisters. All reads from/writes to subregisters are considered the
/// same operation to super registers.
inline bool isSuperReg(const MCRegisterInfo *MCRI, MCRegister Reg) {
  return MCRI->superregs(Reg).empty();
}

inline SmallVector<MCPhysReg> getSuperRegs(const MCRegisterInfo *MCRI) {
  SmallVector<MCPhysReg> SuperRegs;
  for (auto &&RegClass : MCRI->regclasses())
    for (unsigned I = 0; I < RegClass.getNumRegs(); I++) {
      MCRegister Reg = RegClass.getRegister(I);
      if (isSuperReg(MCRI, Reg))
        SuperRegs.push_back(Reg.id());
    }

  sort(SuperRegs.begin(), SuperRegs.end());
  SuperRegs.erase(toolchain::unique(SuperRegs), SuperRegs.end());
  return SuperRegs;
}

inline SmallVector<MCPhysReg> getTrackingRegs(const MCRegisterInfo *MCRI) {
  SmallVector<MCPhysReg> TrackingRegs;
  for (auto Reg : getSuperRegs(MCRI))
    if (!MCRI->isArtificial(Reg) && !MCRI->isConstant(Reg))
      TrackingRegs.push_back(Reg);
  return TrackingRegs;
}

inline MCRegister getSuperReg(const MCRegisterInfo *MCRI, MCRegister Reg) {
  if (isSuperReg(MCRI, Reg))
    return Reg;
  for (auto SuperReg : MCRI->superregs(Reg))
    if (isSuperReg(MCRI, SuperReg))
      return SuperReg;

  llvm_unreachable("Should either be a super reg, or have a super reg");
}

} // namespace vm::core

#endif
