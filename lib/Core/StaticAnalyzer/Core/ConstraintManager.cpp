//===- ConstraintManager.cpp - Constraints on symbolic values. ------------===//
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
//  This file defined the interface to manage constraints on symbolic values.
//
//===----------------------------------------------------------------------===//

#include "language/Core/StaticAnalyzer/Core/PathSensitive/ConstraintManager.h"
#include "language/Core/AST/Type.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/MemRegion.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/ProgramState.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/ProgramState_Fwd.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/SVals.h"
#include "toolchain/ADT/ScopeExit.h"

using namespace language::Core;
using namespace ento;

ConstraintManager::~ConstraintManager() = default;

static DefinedSVal getLocFromSymbol(const ProgramStateRef &State,
                                    SymbolRef Sym) {
  const MemRegion *R =
      State->getStateManager().getRegionManager().getSymbolicRegion(Sym);
  return loc::MemRegionVal(R);
}

ConditionTruthVal ConstraintManager::checkNull(ProgramStateRef State,
                                               SymbolRef Sym) {
  QualType Ty = Sym->getType();
  DefinedSVal V = Loc::isLocType(Ty) ? getLocFromSymbol(State, Sym)
                                     : nonloc::SymbolVal(Sym);
  const ProgramStatePair &P = assumeDual(State, V);
  if (P.first && !P.second)
    return ConditionTruthVal(false);
  if (!P.first && P.second)
    return ConditionTruthVal(true);
  return {};
}

template <typename AssumeFunction>
ConstraintManager::ProgramStatePair
ConstraintManager::assumeDualImpl(ProgramStateRef &State,
                                  AssumeFunction &Assume) {
  if (LLVM_UNLIKELY(State->isPosteriorlyOverconstrained()))
    return {State, State};

  // Assume functions might recurse (see `reAssume` or `tryRearrange`). During
  // the recursion the State might not change anymore, that means we reached a
  // fixpoint.
  // We avoid infinite recursion of assume calls by checking already visited
  // States on the stack of assume function calls.
  const ProgramState *RawSt = State.get();
  if (LLVM_UNLIKELY(AssumeStack.contains(RawSt)))
    return {State, State};
  AssumeStack.push(RawSt);
  auto AssumeStackBuilder =
      toolchain::make_scope_exit([this]() { AssumeStack.pop(); });

  ProgramStateRef StTrue = Assume(true);

  if (!StTrue) {
    ProgramStateRef StFalse = Assume(false);
    if (LLVM_UNLIKELY(!StFalse)) { // both infeasible
      ProgramStateRef StInfeasible = State->cloneAsPosteriorlyOverconstrained();
      assert(StInfeasible->isPosteriorlyOverconstrained());
      // Checkers might rely on the API contract that both returned states
      // cannot be null. Thus, we return StInfeasible for both branches because
      // it might happen that a Checker uncoditionally uses one of them if the
      // other is a nullptr. This may also happen with the non-dual and
      // adjacent `assume(true)` and `assume(false)` calls. By implementing
      // assume in therms of assumeDual, we can keep our API contract there as
      // well.
      return ProgramStatePair(StInfeasible, StInfeasible);
    }
    return ProgramStatePair(nullptr, StFalse);
  }

  ProgramStateRef StFalse = Assume(false);
  if (!StFalse) {
    return ProgramStatePair(StTrue, nullptr);
  }

  return ProgramStatePair(StTrue, StFalse);
}

ConstraintManager::ProgramStatePair
ConstraintManager::assumeDual(ProgramStateRef State, DefinedSVal Cond) {
  auto AssumeFun = [&, Cond](bool Assumption) {
    return assumeInternal(State, Cond, Assumption);
  };
  return assumeDualImpl(State, AssumeFun);
}

ConstraintManager::ProgramStatePair
ConstraintManager::assumeInclusiveRangeDual(ProgramStateRef State, NonLoc Value,
                                            const toolchain::APSInt &From,
                                            const toolchain::APSInt &To) {
  auto AssumeFun = [&](bool Assumption) {
    return assumeInclusiveRangeInternal(State, Value, From, To, Assumption);
  };
  return assumeDualImpl(State, AssumeFun);
}

ProgramStateRef ConstraintManager::assume(ProgramStateRef State,
                                          DefinedSVal Cond, bool Assumption) {
  ConstraintManager::ProgramStatePair R = assumeDual(State, Cond);
  return Assumption ? R.first : R.second;
}

ProgramStateRef
ConstraintManager::assumeInclusiveRange(ProgramStateRef State, NonLoc Value,
                                        const toolchain::APSInt &From,
                                        const toolchain::APSInt &To, bool InBound) {
  ConstraintManager::ProgramStatePair R =
      assumeInclusiveRangeDual(State, Value, From, To);
  return InBound ? R.first : R.second;
}
