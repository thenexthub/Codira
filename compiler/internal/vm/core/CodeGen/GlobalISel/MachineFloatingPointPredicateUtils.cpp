//===- MachineFloatingPointPredicateUtils.cpp -----------------------------===//
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

#include "vm/core/CodeGen/GlobalISel/MachineFloatingPointPredicateUtils.h"
#include "vm/core/CodeGen/GlobalISel/MIPatternMatch.h"
#include "vm/core/CodeGen/LowLevelTypeUtils.h"
#include "vm/core/CodeGen/MachineRegisterInfo.h"
#include "vm/core/CodeGen/MachineSSAContext.h"
#include "vm/core/IR/Constants.h"
#include <optional>

namespace vm::core {

using namespace MIPatternMatch;

template <>
DenormalMode
MachineFloatingPointPredicateUtils::queryDenormalMode(const MachineFunction &MF,
                                                      Register Val) {
  const MachineRegisterInfo &MRI = MF.getRegInfo();
  LLT Ty = MRI.getType(Val).getScalarType();
  return MF.getDenormalMode(getFltSemanticForLLT(Ty));
}

template <>
bool MachineFloatingPointPredicateUtils::lookThroughFAbs(
    const MachineFunction &MF, Register LHS, Register &Src) {
  const MachineRegisterInfo &MRI = MF.getRegInfo();
  return mi_match(LHS, MRI, m_GFabs(m_Reg(Src)));
}

template <>
std::optional<APFloat> MachineFloatingPointPredicateUtils::matchConstantFloat(
    const MachineFunction &MF, Register Val) {
  const MachineRegisterInfo &MRI = MF.getRegInfo();
  const ConstantFP *ConstVal;
  if (mi_match(Val, MRI, m_GFCst(ConstVal)))
    return ConstVal->getValueAPF();

  return std::nullopt;
}

} // namespace vm::core
