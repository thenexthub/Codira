//=== UndefBranchChecker.cpp -----------------------------------*- C++ -*--===//
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
// This file defines UndefBranchChecker, which checks for undefined branch
// condition.
//
//===----------------------------------------------------------------------===//

#include "language/Core/AST/StmtObjC.h"
#include "language/Core/AST/Type.h"
#include "language/Core/StaticAnalyzer/Checkers/BuiltinCheckerRegistration.h"
#include "language/Core/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "language/Core/StaticAnalyzer/Core/Checker.h"
#include "language/Core/StaticAnalyzer/Core/CheckerManager.h"
#include "language/Core/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include <optional>
#include <utility>

using namespace language::Core;
using namespace ento;

namespace {

class UndefBranchChecker : public Checker<check::BranchCondition> {
  const BugType BT{this, "Branch condition evaluates to a garbage value"};

  struct FindUndefExpr {
    ProgramStateRef St;
    const LocationContext *LCtx;

    FindUndefExpr(ProgramStateRef S, const LocationContext *L)
        : St(std::move(S)), LCtx(L) {}

    const Expr *FindExpr(const Expr *Ex) {
      if (!MatchesCriteria(Ex))
        return nullptr;

      for (const Stmt *SubStmt : Ex->children())
        if (const Expr *ExI = dyn_cast_or_null<Expr>(SubStmt))
          if (const Expr *E2 = FindExpr(ExI))
            return E2;

      return Ex;
    }

    bool MatchesCriteria(const Expr *Ex) {
      return St->getSVal(Ex, LCtx).isUndef();
    }
  };

public:
  void checkBranchCondition(const Stmt *Condition, CheckerContext &Ctx) const;
};

} // namespace

void UndefBranchChecker::checkBranchCondition(const Stmt *Condition,
                                              CheckerContext &Ctx) const {
  // ObjCForCollection is a loop, but has no actual condition.
  if (isa<ObjCForCollectionStmt>(Condition))
    return;
  if (!Ctx.getSVal(Condition).isUndef())
    return;

  // Generate a sink node, which implicitly marks both outgoing branches as
  // infeasible.
  ExplodedNode *N = Ctx.generateErrorNode();
  if (!N)
    return;
  // What's going on here: we want to highlight the subexpression of the
  // condition that is the most likely source of the "uninitialized
  // branch condition."  We do a recursive walk of the condition's
  // subexpressions and roughly look for the most nested subexpression
  // that binds to Undefined.  We then highlight that expression's range.

  // Get the predecessor node and check if is a PostStmt with the Stmt
  // being the terminator condition.  We want to inspect the state
  // of that node instead because it will contain main information about
  // the subexpressions.

  // Note: any predecessor will do.  They should have identical state,
  // since all the BlockEdge did was act as an error sink since the value
  // had to already be undefined.
  assert(!N->pred_empty());
  const Expr *Ex = cast<Expr>(Condition);
  ExplodedNode *PrevN = *N->pred_begin();
  ProgramPoint P = PrevN->getLocation();
  ProgramStateRef St = N->getState();

  if (std::optional<PostStmt> PS = P.getAs<PostStmt>())
    if (PS->getStmt() == Ex)
      St = PrevN->getState();

  FindUndefExpr FindIt(St, Ctx.getLocationContext());
  Ex = FindIt.FindExpr(Ex);

  // Emit the bug report.
  auto R = std::make_unique<PathSensitiveBugReport>(BT, BT.getDescription(), N);
  bugreporter::trackExpressionValue(N, Ex, *R);
  R->addRange(Ex->getSourceRange());

  Ctx.emitReport(std::move(R));
}

void ento::registerUndefBranchChecker(CheckerManager &mgr) {
  mgr.registerChecker<UndefBranchChecker>();
}

bool ento::shouldRegisterUndefBranchChecker(const CheckerManager &mgr) {
  return true;
}
