//===--- UndefinedArraySubscriptChecker.h ----------------------*- C++ -*--===//
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
// This defines UndefinedArraySubscriptChecker, a builtin check in ExprEngine
// that performs checks for undefined array subscripts.
//
//===----------------------------------------------------------------------===//

#include "language/Core/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "language/Core/AST/DeclCXX.h"
#include "language/Core/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "language/Core/StaticAnalyzer/Core/Checker.h"
#include "language/Core/StaticAnalyzer/Core/CheckerManager.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"

using namespace language::Core;
using namespace ento;

namespace {
class UndefinedArraySubscriptChecker
  : public Checker< check::PreStmt<ArraySubscriptExpr> > {
  const BugType BT{this, "Array subscript is undefined"};

public:
  void checkPreStmt(const ArraySubscriptExpr *A, CheckerContext &C) const;
};
} // end anonymous namespace

void
UndefinedArraySubscriptChecker::checkPreStmt(const ArraySubscriptExpr *A,
                                             CheckerContext &C) const {
  const Expr *Index = A->getIdx();
  if (!C.getSVal(Index).isUndef())
    return;

  // Sema generates anonymous array variables for copying array struct fields.
  // Don't warn if we're in an implicitly-generated constructor.
  const Decl *D = C.getLocationContext()->getDecl();
  if (const CXXConstructorDecl *Ctor = dyn_cast<CXXConstructorDecl>(D))
    if (Ctor->isDefaulted())
      return;

  ExplodedNode *N = C.generateErrorNode();
  if (!N)
    return;
  // Generate a report for this bug.
  auto R = std::make_unique<PathSensitiveBugReport>(BT, BT.getDescription(), N);
  R->addRange(A->getIdx()->getSourceRange());
  bugreporter::trackExpressionValue(N, A->getIdx(), *R);
  C.emitReport(std::move(R));
}

void ento::registerUndefinedArraySubscriptChecker(CheckerManager &mgr) {
  mgr.registerChecker<UndefinedArraySubscriptChecker>();
}

bool ento::shouldRegisterUndefinedArraySubscriptChecker(const CheckerManager &mgr) {
  return true;
}
