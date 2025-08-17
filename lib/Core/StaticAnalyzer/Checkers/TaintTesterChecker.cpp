//== TaintTesterChecker.cpp ----------------------------------- -*- C++ -*--=//
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
// This checker can be used for testing how taint data is propagated.
//
//===----------------------------------------------------------------------===//

#include "language/Core/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "language/Core/StaticAnalyzer/Checkers/Taint.h"
#include "language/Core/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "language/Core/StaticAnalyzer/Core/Checker.h"
#include "language/Core/StaticAnalyzer/Core/CheckerManager.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"

using namespace language::Core;
using namespace ento;
using namespace taint;

namespace {
class TaintTesterChecker : public Checker<check::PostStmt<Expr>> {
  const BugType BT{this, "Tainted data", "General"};

public:
  void checkPostStmt(const Expr *E, CheckerContext &C) const;
};
}

void TaintTesterChecker::checkPostStmt(const Expr *E,
                                       CheckerContext &C) const {
  ProgramStateRef State = C.getState();
  if (!State)
    return;

  if (isTainted(State, E, C.getLocationContext())) {
    if (ExplodedNode *N = C.generateNonFatalErrorNode()) {
      auto report = std::make_unique<PathSensitiveBugReport>(BT, "tainted", N);
      report->addRange(E->getSourceRange());
      C.emitReport(std::move(report));
    }
  }
}

void ento::registerTaintTesterChecker(CheckerManager &mgr) {
  mgr.registerChecker<TaintTesterChecker>();
}

bool ento::shouldRegisterTaintTesterChecker(const CheckerManager &mgr) {
  return true;
}
