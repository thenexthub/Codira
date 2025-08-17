//===- ConstraintManager.h - Constraints on symbolic values. ----*- C++ -*-===//
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

#ifndef LANGUAGE_CORE_STATICANALYZER_CORE_PATHSENSITIVE_CONSTRAINTMANAGER_H
#define LANGUAGE_CORE_STATICANALYZER_CORE_PATHSENSITIVE_CONSTRAINTMANAGER_H

#include "language/Core/Basic/LLVM.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/ProgramState_Fwd.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/SVals.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/SymExpr.h"
#include "toolchain/Support/SaveAndRestore.h"
#include <memory>
#include <optional>
#include <utility>

namespace toolchain {

class APSInt;

} // namespace toolchain

namespace language::Core {
namespace ento {

class ProgramStateManager;
class ExprEngine;
class SymbolReaper;

class ConditionTruthVal {
  std::optional<bool> Val;

public:
  /// Construct a ConditionTruthVal indicating the constraint is constrained
  /// to either true or false, depending on the boolean value provided.
  ConditionTruthVal(bool constraint) : Val(constraint) {}

  /// Construct a ConstraintVal indicating the constraint is underconstrained.
  ConditionTruthVal() = default;

  /// \return Stored value, assuming that the value is known.
  /// Crashes otherwise.
  bool getValue() const {
    return *Val;
  }

  /// Return true if the constraint is perfectly constrained to 'true'.
  bool isConstrainedTrue() const { return Val && *Val; }

  /// Return true if the constraint is perfectly constrained to 'false'.
  bool isConstrainedFalse() const { return Val && !*Val; }

  /// Return true if the constrained is perfectly constrained.
  bool isConstrained() const { return Val.has_value(); }

  /// Return true if the constrained is underconstrained and we do not know
  /// if the constraint is true of value.
  bool isUnderconstrained() const { return !Val.has_value(); }
};

class ConstraintManager {
public:
  ConstraintManager() = default;
  virtual ~ConstraintManager();

  virtual bool haveEqualConstraints(ProgramStateRef S1,
                                    ProgramStateRef S2) const = 0;

  ProgramStateRef assume(ProgramStateRef state, DefinedSVal Cond,
                         bool Assumption);

  using ProgramStatePair = std::pair<ProgramStateRef, ProgramStateRef>;

  /// Returns a pair of states (StTrue, StFalse) where the given condition is
  /// assumed to be true or false, respectively.
  /// (Note that these two states might be equal if the parent state turns out
  /// to be infeasible. This may happen if the underlying constraint solver is
  /// not perfectly precise and this may happen very rarely.)
  ProgramStatePair assumeDual(ProgramStateRef State, DefinedSVal Cond);

  ProgramStateRef assumeInclusiveRange(ProgramStateRef State, NonLoc Value,
                                       const toolchain::APSInt &From,
                                       const toolchain::APSInt &To, bool InBound);

  /// Returns a pair of states (StInRange, StOutOfRange) where the given value
  /// is assumed to be in the range or out of the range, respectively.
  /// (Note that these two states might be equal if the parent state turns out
  /// to be infeasible. This may happen if the underlying constraint solver is
  /// not perfectly precise and this may happen very rarely.)
  ProgramStatePair assumeInclusiveRangeDual(ProgramStateRef State, NonLoc Value,
                                            const toolchain::APSInt &From,
                                            const toolchain::APSInt &To);

  /// If a symbol is perfectly constrained to a constant, attempt
  /// to return the concrete value.
  ///
  /// Note that a ConstraintManager is not obligated to return a concretized
  /// value for a symbol, even if it is perfectly constrained.
  /// It might return null.
  virtual const toolchain::APSInt* getSymVal(ProgramStateRef state,
                                        SymbolRef sym) const {
    return nullptr;
  }

  /// Attempt to return the minimal possible value for a given symbol. Note
  /// that a ConstraintManager is not obligated to return a lower bound, it may
  /// also return nullptr.
  virtual const toolchain::APSInt *getSymMinVal(ProgramStateRef state,
                                           SymbolRef sym) const {
    return nullptr;
  }

  /// Attempt to return the minimal possible value for a given symbol. Note
  /// that a ConstraintManager is not obligated to return a lower bound, it may
  /// also return nullptr.
  virtual const toolchain::APSInt *getSymMaxVal(ProgramStateRef state,
                                           SymbolRef sym) const {
    return nullptr;
  }

  /// Scan all symbols referenced by the constraints. If the symbol is not
  /// alive, remove it.
  virtual ProgramStateRef removeDeadBindings(ProgramStateRef state,
                                                 SymbolReaper& SymReaper) = 0;

  virtual void printJson(raw_ostream &Out, ProgramStateRef State,
                         const char *NL, unsigned int Space,
                         bool IsDot) const = 0;

  virtual void printValue(raw_ostream &Out, ProgramStateRef State,
                          SymbolRef Sym) {}

  /// Convenience method to query the state to see if a symbol is null or
  /// not null, or if neither assumption can be made.
  ConditionTruthVal isNull(ProgramStateRef State, SymbolRef Sym) {
    return checkNull(State, Sym);
  }

protected:
  /// A helper class to simulate the call stack of nested assume calls.
  class AssumeStackTy {
  public:
    void push(const ProgramState *S) { Aux.push_back(S); }
    void pop() { Aux.pop_back(); }
    bool contains(const ProgramState *S) const {
      return toolchain::is_contained(Aux, S);
    }

  private:
    toolchain::SmallVector<const ProgramState *, 4> Aux;
  };
  AssumeStackTy AssumeStack;

  virtual ProgramStateRef assumeInternal(ProgramStateRef state,
                                         DefinedSVal Cond, bool Assumption) = 0;

  virtual ProgramStateRef assumeInclusiveRangeInternal(ProgramStateRef State,
                                                       NonLoc Value,
                                                       const toolchain::APSInt &From,
                                                       const toolchain::APSInt &To,
                                                       bool InBound) = 0;

  /// canReasonAbout - Not all ConstraintManagers can accurately reason about
  ///  all SVal values.  This method returns true if the ConstraintManager can
  ///  reasonably handle a given SVal value.  This is typically queried by
  ///  ExprEngine to determine if the value should be replaced with a
  ///  conjured symbolic value in order to recover some precision.
  virtual bool canReasonAbout(SVal X) const = 0;

  /// Returns whether or not a symbol is known to be null ("true"), known to be
  /// non-null ("false"), or may be either ("underconstrained").
  virtual ConditionTruthVal checkNull(ProgramStateRef State, SymbolRef Sym);

  template <typename AssumeFunction>
  ProgramStatePair assumeDualImpl(ProgramStateRef &State,
                                  AssumeFunction &Assume);
};

std::unique_ptr<ConstraintManager>
CreateRangeConstraintManager(ProgramStateManager &statemgr,
                             ExprEngine *exprengine);

std::unique_ptr<ConstraintManager>
CreateZ3ConstraintManager(ProgramStateManager &statemgr,
                          ExprEngine *exprengine);

} // namespace ento
} // namespace language::Core

#endif // LANGUAGE_CORE_STATICANALYZER_CORE_PATHSENSITIVE_CONSTRAINTMANAGER_H
