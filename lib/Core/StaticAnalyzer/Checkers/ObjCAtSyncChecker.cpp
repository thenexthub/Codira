//== ObjCAtSyncChecker.cpp - nil mutex checker for @synchronized -*- C++ -*--=//
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
// This defines ObjCAtSyncChecker, a builtin check that checks for null pointers
// used as mutexes for @synchronized.
//
//===----------------------------------------------------------------------===//

#include "language/Core/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "language/Core/AST/StmtObjC.h"
#include "language/Core/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "language/Core/StaticAnalyzer/Core/Checker.h"
#include "language/Core/StaticAnalyzer/Core/CheckerManager.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/ExprEngine.h"

using namespace language::Core;
using namespace ento;

namespace {
class ObjCAtSyncChecker
    : public Checker< check::PreStmt<ObjCAtSynchronizedStmt> > {
  const BugType BT_null{this, "Nil value used as mutex for @synchronized() "
                              "(no synchronization will occur)"};
  const BugType BT_undef{this, "Uninitialized value used as mutex "
                               "for @synchronized"};

public:
  void checkPreStmt(const ObjCAtSynchronizedStmt *S, CheckerContext &C) const;
};
} // end anonymous namespace

void ObjCAtSyncChecker::checkPreStmt(const ObjCAtSynchronizedStmt *S,
                                     CheckerContext &C) const {

  const Expr *Ex = S->getSynchExpr();
  ProgramStateRef state = C.getState();
  SVal V = C.getSVal(Ex);

  // Uninitialized value used for the mutex?
  if (isa<UndefinedVal>(V)) {
    if (ExplodedNode *N = C.generateErrorNode()) {
      auto report = std::make_unique<PathSensitiveBugReport>(
          BT_undef, BT_undef.getDescription(), N);
      bugreporter::trackExpressionValue(N, Ex, *report);
      C.emitReport(std::move(report));
    }
    return;
  }

  if (V.isUnknown())
    return;

  // Check for null mutexes.
  ProgramStateRef notNullState, nullState;
  std::tie(notNullState, nullState) = state->assume(V.castAs<DefinedSVal>());

  if (nullState) {
    if (!notNullState) {
      // Generate an error node.  This isn't a sink since
      // a null mutex just means no synchronization occurs.
      if (ExplodedNode *N = C.generateNonFatalErrorNode(nullState)) {
        auto report = std::make_unique<PathSensitiveBugReport>(
            BT_null, BT_null.getDescription(), N);
        bugreporter::trackExpressionValue(N, Ex, *report);

        C.emitReport(std::move(report));
        return;
      }
    }
    // Don't add a transition for 'nullState'.  If the value is
    // under-constrained to be null or non-null, assume it is non-null
    // afterwards.
  }

  if (notNullState)
    C.addTransition(notNullState);
}

void ento::registerObjCAtSyncChecker(CheckerManager &mgr) {
  mgr.registerChecker<ObjCAtSyncChecker>();
}

bool ento::shouldRegisterObjCAtSyncChecker(const CheckerManager &mgr) {
  const LangOptions &LO = mgr.getLangOpts();
  return LO.ObjC;
}
