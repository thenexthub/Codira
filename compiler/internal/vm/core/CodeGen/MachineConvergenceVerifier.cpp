//===- MachineConvergenceVerifier.cpp - Verify convergencectrl ------------===//
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
//===----------------------------------------------------------------------===//

#include "vm/core/CodeGen/MachineConvergenceVerifier.h"
#include "vm/core/CodeGen/MachineDominators.h"
#include "vm/core/CodeGen/MachineRegisterInfo.h"
#include "vm/core/CodeGen/MachineSSAContext.h"
#include "vm/core/IR/GenericConvergenceVerifierImpl.h"

using namespace vm::core;

template <>
auto GenericConvergenceVerifier<MachineSSAContext>::getConvOp(
    const MachineInstr &MI) -> ConvOpKind {
  switch (MI.getOpcode()) {
  default:
    return CONV_NONE;
  case TargetOpcode::CONVERGENCECTRL_ENTRY:
    return CONV_ENTRY;
  case TargetOpcode::CONVERGENCECTRL_ANCHOR:
    return CONV_ANCHOR;
  case TargetOpcode::CONVERGENCECTRL_LOOP:
    return CONV_LOOP;
  }
}

template <>
void GenericConvergenceVerifier<
    MachineSSAContext>::checkConvergenceTokenProduced(const MachineInstr &MI) {
  Check(!MI.hasImplicitDef(),
        "Convergence control tokens are defined explicitly.",
        {Context.print(&MI)});
  const MachineOperand &Def = MI.getOperand(0);
  const MachineRegisterInfo &MRI = Context.getFunction()->getRegInfo();
  Check(MRI.getUniqueVRegDef(Def.getReg()),
        "Convergence control tokens must have unique definitions.",
        {Context.print(&MI)});
}

template <>
const MachineInstr *
GenericConvergenceVerifier<MachineSSAContext>::findAndCheckConvergenceTokenUsed(
    const MachineInstr &MI) {
  const MachineRegisterInfo &MRI = Context.getFunction()->getRegInfo();
  const MachineInstr *TokenDef = nullptr;

  for (const MachineOperand &MO : MI.operands()) {
    if (!MO.isReg() || !MO.isUse())
      continue;
    Register OpReg = MO.getReg();
    if (!OpReg.isVirtual())
      continue;

    const MachineInstr *Def = MRI.getUniqueVRegDef(OpReg);
    if (!Def)
      continue;
    if (getConvOp(*Def) == CONV_NONE)
      continue;

    CheckOrNull(
        MI.isConvergent(),
        "Convergence control tokens can only be used by convergent operations.",
        {Context.print(OpReg), Context.print(&MI)});

    CheckOrNull(!TokenDef,
                "An operation can use at most one convergence control token.",
                {Context.print(OpReg), Context.print(&MI)});

    TokenDef = Def;
  }

  if (TokenDef)
    Tokens[&MI] = TokenDef;

  return TokenDef;
}

template <>
bool GenericConvergenceVerifier<MachineSSAContext>::isInsideConvergentFunction(
    const MachineInstr &MI) {
  // The class MachineFunction does not have any property to indicate whether it
  // is convergent. Trivially return true so that the check always passes.
  return true;
}

template <>
bool GenericConvergenceVerifier<MachineSSAContext>::isConvergent(
    const MachineInstr &MI) {
  return MI.isConvergent();
}

template class toolchain::GenericConvergenceVerifier<MachineSSAContext>;
