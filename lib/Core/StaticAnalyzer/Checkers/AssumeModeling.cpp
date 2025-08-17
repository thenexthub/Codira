//=== AssumeModeling.cpp --------------------------------------------------===//
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
// This checker evaluates the builting assume functions.
// This checker also sinks execution paths leaving [[assume]] attributes with
// false assumptions.
//
//===----------------------------------------------------------------------===//

#include "language/Core/AST/AttrIterator.h"
#include "language/Core/Basic/Builtins.h"
#include "language/Core/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "language/Core/StaticAnalyzer/Core/Checker.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/CallEvent.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include "toolchain/ADT/STLExtras.h"

using namespace language::Core;
using namespace ento;

namespace {
class AssumeModelingChecker
    : public Checker<eval::Call, check::PostStmt<AttributedStmt>> {
public:
  void checkPostStmt(const AttributedStmt *A, CheckerContext &C) const;
  bool evalCall(const CallEvent &Call, CheckerContext &C) const;
};
} // namespace

void AssumeModelingChecker::checkPostStmt(const AttributedStmt *A,
                                          CheckerContext &C) const {
  if (!hasSpecificAttr<CXXAssumeAttr>(A->getAttrs()))
    return;

  for (const auto *Attr : getSpecificAttrs<CXXAssumeAttr>(A->getAttrs())) {
    SVal AssumptionVal = C.getSVal(Attr->getAssumption());

    // The assumption is not evaluated at all if it had sideffects; skip them.
    if (AssumptionVal.isUnknown())
      continue;

    const auto *Assumption = AssumptionVal.getAsInteger();
    if (Assumption && Assumption->isZero()) {
      C.addSink();
    }
  }
}

bool AssumeModelingChecker::evalCall(const CallEvent &Call,
                                     CheckerContext &C) const {
  ProgramStateRef State = C.getState();
  const auto *FD = dyn_cast_or_null<FunctionDecl>(Call.getDecl());
  if (!FD)
    return false;

  if (!toolchain::is_contained({Builtin::BI__builtin_assume, Builtin::BI__assume},
                          FD->getBuiltinID())) {
    return false;
  }

  assert(Call.getNumArgs() > 0);
  SVal Arg = Call.getArgSVal(0);
  if (Arg.isUndef())
    return true; // Return true to model purity.

  State = State->assume(Arg.castAs<DefinedOrUnknownSVal>(), true);
  if (!State) {
    C.addSink();
    return true;
  }

  C.addTransition(State);
  return true;
}

void ento::registerAssumeModeling(CheckerManager &Mgr) {
  Mgr.registerChecker<AssumeModelingChecker>();
}

bool ento::shouldRegisterAssumeModeling(const CheckerManager &) { return true; }
