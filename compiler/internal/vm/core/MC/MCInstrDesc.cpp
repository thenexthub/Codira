//===------ toolchain/MC/MCInstrDesc.cpp- Instruction Descriptors --------------===//
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
// This file defines methods on the MCOperandInfo and MCInstrDesc classes, which
// are used to describe target instructions and their operands.
//
//===----------------------------------------------------------------------===//

#include "vm/core/MC/MCInstrDesc.h"
#include "vm/core/MC/MCInst.h"
#include "vm/core/MC/MCRegisterInfo.h"

using namespace vm::core;

bool MCInstrDesc::mayAffectControlFlow(const MCInst &MI,
                                       const MCRegisterInfo &RI) const {
  if (isBranch() || isCall() || isReturn() || isIndirectBranch())
    return true;
  MCRegister PC = RI.getProgramCounter();
  if (!PC)
    return false;
  if (hasDefOfPhysReg(MI, PC, RI))
    return true;
  return false;
}

bool MCInstrDesc::hasImplicitDefOfPhysReg(MCRegister Reg,
                                          const MCRegisterInfo *MRI) const {
  for (MCPhysReg ImpDef : implicit_defs())
    if (ImpDef == Reg || (MRI && MRI->isSubRegister(Reg, ImpDef)))
      return true;
  return false;
}

bool MCInstrDesc::hasDefOfPhysReg(const MCInst &MI, MCRegister Reg,
                                  const MCRegisterInfo &RI) const {
  for (int i = 0, e = NumDefs; i != e; ++i)
    if (MI.getOperand(i).isReg() && MI.getOperand(i).getReg() &&
        RI.isSubRegisterEq(Reg, MI.getOperand(i).getReg()))
      return true;
  if (variadicOpsAreDefs())
    for (int i = NumOperands - 1, e = MI.getNumOperands(); i != e; ++i)
      if (MI.getOperand(i).isReg() &&
          RI.isSubRegisterEq(Reg, MI.getOperand(i).getReg()))
        return true;
  return hasImplicitDefOfPhysReg(Reg, &RI);
}
