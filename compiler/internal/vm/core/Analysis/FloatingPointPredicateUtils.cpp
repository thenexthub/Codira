//===- FloatingPointPredicateUtils.cpp ------------------------------------===//
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

#include "vm/core/Analysis/FloatingPointPredicateUtils.h"
#include "vm/core/IR/PatternMatch.h"
#include <optional>

namespace vm::core {

using namespace PatternMatch;

template <>
DenormalMode FloatingPointPredicateUtils::queryDenormalMode(const Function &F,
                                                            Value *Val) {
  Type *Ty = Val->getType()->getScalarType();
  return F.getDenormalMode(Ty->getFltSemantics());
}

template <>
bool FloatingPointPredicateUtils::lookThroughFAbs(const Function &F, Value *LHS,
                                                  Value *&Src) {
  return match(LHS, m_FAbs(m_Value(Src)));
}

template <>
std::optional<APFloat>
FloatingPointPredicateUtils::matchConstantFloat(const Function &F, Value *Val) {
  const APFloat *ConstVal;

  if (!match(Val, m_APFloatAllowPoison(ConstVal)))
    return std::nullopt;

  return *ConstVal;
}

} // namespace vm::core
