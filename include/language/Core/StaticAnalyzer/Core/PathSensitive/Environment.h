//===- Environment.h - Map from Stmt* to Locations/Values -------*- C++ -*-===//
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
//  This file defined the Environment and EnvironmentManager classes.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_STATICANALYZER_CORE_PATHSENSITIVE_ENVIRONMENT_H
#define LANGUAGE_CORE_STATICANALYZER_CORE_PATHSENSITIVE_ENVIRONMENT_H

#include "language/Core/Analysis/AnalysisDeclContext.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/ProgramState_Fwd.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/SVals.h"
#include "toolchain/ADT/ImmutableMap.h"
#include <utility>

namespace language::Core {

class Stmt;

namespace ento {

class SValBuilder;
class SymbolReaper;

/// An entry in the environment consists of a Stmt and an LocationContext.
/// This allows the environment to manage context-sensitive bindings,
/// which is essentially for modeling recursive function analysis, among
/// other things.
class EnvironmentEntry : public std::pair<const Stmt *,
                                          const StackFrameContext *> {
public:
  EnvironmentEntry(const Stmt *s, const LocationContext *L);

  const Stmt *getStmt() const { return first; }
  const LocationContext *getLocationContext() const { return second; }

  /// Profile an EnvironmentEntry for inclusion in a FoldingSet.
  static void Profile(toolchain::FoldingSetNodeID &ID,
                      const EnvironmentEntry &E) {
    ID.AddPointer(E.getStmt());
    ID.AddPointer(E.getLocationContext());
  }

  void Profile(toolchain::FoldingSetNodeID &ID) const {
    Profile(ID, *this);
  }
};

/// An immutable map from EnvironemntEntries to SVals.
class Environment {
private:
  friend class EnvironmentManager;

  using BindingsTy = toolchain::ImmutableMap<EnvironmentEntry, SVal>;

  BindingsTy ExprBindings;

  Environment(BindingsTy eb) : ExprBindings(eb) {}

  SVal lookupExpr(const EnvironmentEntry &E) const;

public:
  using iterator = BindingsTy::iterator;

  iterator begin() const { return ExprBindings.begin(); }
  iterator end() const { return ExprBindings.end(); }

  /// Fetches the current binding of the expression in the
  /// Environment.
  SVal getSVal(const EnvironmentEntry &E, SValBuilder &svalBuilder) const;

  /// Profile - Profile the contents of an Environment object for use
  ///  in a FoldingSet.
  static void Profile(toolchain::FoldingSetNodeID& ID, const Environment* env) {
    env->ExprBindings.Profile(ID);
  }

  /// Profile - Used to profile the contents of this object for inclusion
  ///  in a FoldingSet.
  void Profile(toolchain::FoldingSetNodeID& ID) const {
    Profile(ID, this);
  }

  bool operator==(const Environment& RHS) const {
    return ExprBindings == RHS.ExprBindings;
  }

  void printJson(raw_ostream &Out, const ASTContext &Ctx,
                 const LocationContext *LCtx = nullptr, const char *NL = "\n",
                 unsigned int Space = 0, bool IsDot = false) const;
};

class EnvironmentManager {
private:
  using FactoryTy = Environment::BindingsTy::Factory;

  FactoryTy F;

public:
  EnvironmentManager(toolchain::BumpPtrAllocator &Allocator) : F(Allocator) {}

  Environment getInitialEnvironment() {
    return Environment(F.getEmptyMap());
  }

  /// Bind a symbolic value to the given environment entry.
  Environment bindExpr(Environment Env, const EnvironmentEntry &E, SVal V,
                       bool Invalidate);

  Environment removeDeadBindings(Environment Env,
                                 SymbolReaper &SymReaper,
                                 ProgramStateRef state);
};

} // namespace ento

} // namespace language::Core

#endif // LANGUAGE_CORE_STATICANALYZER_CORE_PATHSENSITIVE_ENVIRONMENT_H
