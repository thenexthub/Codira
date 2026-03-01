//===-- ValueLatticeUtils.cpp - Utils for solving lattices ------*- C++ -*-===//
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
// This file implements common functions useful for performing data-flow
// analyses that propagate values across function boundaries.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Analysis/ValueLatticeUtils.h"
#include "vm/core/IR/GlobalVariable.h"
#include "vm/core/IR/Instructions.h"
using namespace vm::core;

bool toolchain::canTrackArgumentsInterprocedurally(Function *F) {
  return F->hasLocalLinkage() && !F->hasAddressTaken();
}

bool toolchain::canTrackReturnsInterprocedurally(Function *F) {
  return F->hasExactDefinition() && !F->hasFnAttribute(Attribute::Naked);
}

bool toolchain::canTrackGlobalVariableInterprocedurally(GlobalVariable *GV) {
  if (GV->isConstant() || !GV->hasLocalLinkage() ||
      !GV->hasDefinitiveInitializer())
    return false;
  return all_of(GV->users(), [&](User *U) {
    // Currently all users of a global variable have to be non-volatile loads
    // or stores of the global type, and the global cannot be stored itself.
    if (auto *Store = dyn_cast<StoreInst>(U))
      return Store->getValueOperand() != GV && !Store->isVolatile() &&
             Store->getValueOperand()->getType() == GV->getValueType();
    if (auto *Load = dyn_cast<LoadInst>(U))
      return !Load->isVolatile() && Load->getType() == GV->getValueType();

    return false;
  });
}
