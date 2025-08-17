//== ReturnUndefChecker.cpp -------------------------------------*- C++ -*--==//
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
// This file defines ReturnUndefChecker, which is a path-sensitive
// check which looks for undefined or garbage values being returned to the
// caller.
//
//===----------------------------------------------------------------------===//

#include "language/Core/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "language/Core/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "language/Core/StaticAnalyzer/Core/Checker.h"
#include "language/Core/StaticAnalyzer/Core/CheckerManager.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/CallEvent.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"

using namespace language::Core;
using namespace ento;

namespace {
class ReturnUndefChecker : public Checker< check::PreStmt<ReturnStmt> > {
  const BugType BT_Undef{this, "Garbage return value"};
  const BugType BT_NullReference{this, "Returning null reference"};

  void emitUndef(CheckerContext &C, const Expr *RetE) const;
  void checkReference(CheckerContext &C, const Expr *RetE,
                      DefinedOrUnknownSVal RetVal) const;
public:
  void checkPreStmt(const ReturnStmt *RS, CheckerContext &C) const;
};
}

void ReturnUndefChecker::checkPreStmt(const ReturnStmt *RS,
                                      CheckerContext &C) const {
  const Expr *RetE = RS->getRetValue();
  if (!RetE)
    return;
  SVal RetVal = C.getSVal(RetE);

  const StackFrameContext *SFC = C.getStackFrame();
  QualType RT = CallEvent::getDeclaredResultType(SFC->getDecl());

  if (RetVal.isUndef()) {
    // "return;" is modeled to evaluate to an UndefinedVal. Allow UndefinedVal
    // to be returned in functions returning void to support this pattern:
    //   void foo() {
    //     return;
    //   }
    //   void test() {
    //     return foo();
    //   }
    if (!RT.isNull() && RT->isVoidType())
      return;

    // Not all blocks have explicitly-specified return types; if the return type
    // is not available, but the return value expression has 'void' type, assume
    // Sema already checked it.
    if (RT.isNull() && isa<BlockDecl>(SFC->getDecl()) &&
        RetE->getType()->isVoidType())
      return;

    emitUndef(C, RetE);
    return;
  }

  if (RT.isNull())
    return;

  if (RT->isReferenceType()) {
    checkReference(C, RetE, RetVal.castAs<DefinedOrUnknownSVal>());
    return;
  }
}

static void emitBug(CheckerContext &C, const BugType &BT, StringRef Msg,
                    const Expr *RetE, const Expr *TrackingE = nullptr) {
  ExplodedNode *N = C.generateErrorNode();
  if (!N)
    return;

  auto Report = std::make_unique<PathSensitiveBugReport>(BT, Msg, N);

  Report->addRange(RetE->getSourceRange());
  bugreporter::trackExpressionValue(N, TrackingE ? TrackingE : RetE, *Report);

  C.emitReport(std::move(Report));
}

void ReturnUndefChecker::emitUndef(CheckerContext &C, const Expr *RetE) const {
  emitBug(C, BT_Undef, "Undefined or garbage value returned to caller", RetE);
}

void ReturnUndefChecker::checkReference(CheckerContext &C, const Expr *RetE,
                                        DefinedOrUnknownSVal RetVal) const {
  ProgramStateRef StNonNull, StNull;
  std::tie(StNonNull, StNull) = C.getState()->assume(RetVal);

  if (StNonNull) {
    // Going forward, assume the location is non-null.
    C.addTransition(StNonNull);
    return;
  }

  // The return value is known to be null. Emit a bug report.
  emitBug(C, BT_NullReference, BT_NullReference.getDescription(), RetE,
          bugreporter::getDerefExpr(RetE));
}

void ento::registerReturnUndefChecker(CheckerManager &mgr) {
  mgr.registerChecker<ReturnUndefChecker>();
}

bool ento::shouldRegisterReturnUndefChecker(const CheckerManager &mgr) {
  return true;
}
