//== SimpleConstraintManager.cpp --------------------------------*- C++ -*--==//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
// 
// Author: Tunjay Akbarli
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
// 
//     http://www.apache.org/licenses/LICENSE-2.0
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
//  This file defines SimpleConstraintManager, a class that provides a
//  simplified constraint manager interface, compared to ConstraintManager.
//
//===----------------------------------------------------------------------===//

#include "language/Core/StaticAnalyzer/Core/PathSensitive/SimpleConstraintManager.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/ExprEngine.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/ProgramState.h"
#include <optional>

namespace language::Core {

namespace ento {

SimpleConstraintManager::~SimpleConstraintManager() {}

ProgramStateRef SimpleConstraintManager::assumeInternal(ProgramStateRef State,
                                                        DefinedSVal Cond,
                                                        bool Assumption) {
  // If we have a Loc value, cast it to a bool NonLoc first.
  if (std::optional<Loc> LV = Cond.getAs<Loc>()) {
    SValBuilder &SVB = State->getStateManager().getSValBuilder();
    QualType T;
    const MemRegion *MR = LV->getAsRegion();
    if (const TypedRegion *TR = dyn_cast_or_null<TypedRegion>(MR))
      T = TR->getLocationType();
    else
      T = SVB.getContext().VoidPtrTy;

    Cond = SVB.evalCast(*LV, SVB.getContext().BoolTy, T).castAs<DefinedSVal>();
  }

  return assume(State, Cond.castAs<NonLoc>(), Assumption);
}

ProgramStateRef SimpleConstraintManager::assume(ProgramStateRef State,
                                                NonLoc Cond, bool Assumption) {
  State = assumeAux(State, Cond, Assumption);
  if (EE)
    return EE->processAssume(State, Cond, Assumption);
  return State;
}

ProgramStateRef SimpleConstraintManager::assumeAux(ProgramStateRef State,
                                                   NonLoc Cond,
                                                   bool Assumption) {

  // We cannot reason about SymSymExprs, and can only reason about some
  // SymIntExprs.
  if (!canReasonAbout(Cond)) {
    // Just add the constraint to the expression without trying to simplify.
    SymbolRef Sym = Cond.getAsSymbol();
    assert(Sym);
    return assumeSymUnsupported(State, Sym, Assumption);
  }

  switch (Cond.getKind()) {
  default:
    toolchain_unreachable("'Assume' not implemented for this NonLoc");

  case nonloc::SymbolValKind: {
    nonloc::SymbolVal SV = Cond.castAs<nonloc::SymbolVal>();
    SymbolRef Sym = SV.getSymbol();
    assert(Sym);
    return assumeSym(State, Sym, Assumption);
  }

  case nonloc::ConcreteIntKind: {
    bool b = *Cond.castAs<nonloc::ConcreteInt>().getValue() != 0;
    bool isFeasible = b ? Assumption : !Assumption;
    return isFeasible ? State : nullptr;
  }

  case nonloc::PointerToMemberKind: {
    bool IsNull = !Cond.castAs<nonloc::PointerToMember>().isNullMemberPointer();
    bool IsFeasible = IsNull ? Assumption : !Assumption;
    return IsFeasible ? State : nullptr;
  }

  case nonloc::LocAsIntegerKind:
    return assumeInternal(State, Cond.castAs<nonloc::LocAsInteger>().getLoc(),
                          Assumption);
  } // end switch
}

ProgramStateRef SimpleConstraintManager::assumeInclusiveRangeInternal(
    ProgramStateRef State, NonLoc Value, const toolchain::APSInt &From,
    const toolchain::APSInt &To, bool InRange) {

  assert(From.isUnsigned() == To.isUnsigned() &&
         From.getBitWidth() == To.getBitWidth() &&
         "Values should have same types!");

  if (!canReasonAbout(Value)) {
    // Just add the constraint to the expression without trying to simplify.
    SymbolRef Sym = Value.getAsSymbol();
    assert(Sym);
    return assumeSymInclusiveRange(State, Sym, From, To, InRange);
  }

  switch (Value.getKind()) {
  default:
    toolchain_unreachable("'assumeInclusiveRange' is not implemented"
                     "for this NonLoc");

  case nonloc::LocAsIntegerKind:
  case nonloc::SymbolValKind: {
    if (SymbolRef Sym = Value.getAsSymbol())
      return assumeSymInclusiveRange(State, Sym, From, To, InRange);
    return State;
  } // end switch

  case nonloc::ConcreteIntKind: {
    const toolchain::APSInt &IntVal = Value.castAs<nonloc::ConcreteInt>().getValue();
    bool IsInRange = IntVal >= From && IntVal <= To;
    bool isFeasible = (IsInRange == InRange);
    return isFeasible ? State : nullptr;
  }
  } // end switch
}

} // end of namespace ento

} // end of namespace language::Core
