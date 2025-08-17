//== PutenvStackArrayChecker.cpp ------------------------------- -*- C++ -*--=//
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
// This file defines PutenvStackArrayChecker which finds calls of ``putenv``
// function with automatic array variable as the argument.
// https://wiki.sei.cmu.edu/confluence/x/6NYxBQ
//
//===----------------------------------------------------------------------===//

#include "language/Core/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "language/Core/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "language/Core/StaticAnalyzer/Core/Checker.h"
#include "language/Core/StaticAnalyzer/Core/CheckerManager.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/CallDescription.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/CallEvent.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/MemRegion.h"

using namespace language::Core;
using namespace ento;

namespace {
class PutenvStackArrayChecker : public Checker<check::PostCall> {
private:
  BugType BT{this, "'putenv' called with stack-allocated string",
             categories::SecurityError};
  const CallDescription Putenv{CDM::CLibrary, {"putenv"}, 1};

public:
  void checkPostCall(const CallEvent &Call, CheckerContext &C) const;
};
} // namespace

void PutenvStackArrayChecker::checkPostCall(const CallEvent &Call,
                                            CheckerContext &C) const {
  if (!Putenv.matches(Call))
    return;

  SVal ArgV = Call.getArgSVal(0);
  const Expr *ArgExpr = Call.getArgExpr(0);

  if (!ArgV.getAsRegion())
    return;

  const auto *SSR =
      ArgV.getAsRegion()->getMemorySpaceAs<StackSpaceRegion>(C.getState());
  if (!SSR)
    return;
  const auto *StackFrameFuncD =
      dyn_cast_or_null<FunctionDecl>(SSR->getStackFrame()->getDecl());
  if (StackFrameFuncD && StackFrameFuncD->isMain())
    return;

  StringRef ErrorMsg = "The 'putenv' function should not be called with "
                       "arrays that have automatic storage";
  ExplodedNode *N = C.generateErrorNode();
  auto Report = std::make_unique<PathSensitiveBugReport>(BT, ErrorMsg, N);

  // Track the argument.
  bugreporter::trackExpressionValue(Report->getErrorNode(), ArgExpr, *Report);

  C.emitReport(std::move(Report));
}

void ento::registerPutenvStackArray(CheckerManager &Mgr) {
  Mgr.registerChecker<PutenvStackArrayChecker>();
}

bool ento::shouldRegisterPutenvStackArray(const CheckerManager &) {
  return true;
}
