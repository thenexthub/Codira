//===--- AfterPoundExprCompletion.cpp -------------------------------------===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//

//===----------------------------------------------------------------------===//

#include "language/IDE/AfterPoundExprCompletion.h"
#include "language/IDE/CodeCompletion.h"
#include "language/IDE/CompletionLookup.h"
#include "language/Sema/CompletionContextFinder.h"
#include "language/Sema/ConstraintSystem.h"
#include "language/Sema/IDETypeChecking.h"

using namespace language;
using namespace language::constraints;
using namespace language::ide;

void AfterPoundExprCompletion::sawSolutionImpl(const constraints::Solution &S) {
  Type ExpectedTy = getTypeForCompletion(S, CompletionExpr);

  bool IsAsync = isContextAsync(S, DC);

  // If ExpectedTy is a duplicate of any other result, ignore this solution.
  auto IsEqual = [&](const Result &R) {
    return R.ExpectedTy->isEqual(ExpectedTy);
  };
  if (!toolchain::any_of(Results, IsEqual)) {
    bool IsImpliedResult = isImpliedResult(S, CompletionExpr);
    Results.push_back({ExpectedTy, IsImpliedResult, IsAsync});
  }
}

void AfterPoundExprCompletion::collectResults(
    ide::CodeCompletionContext &CompletionCtx) {
  ASTContext &Ctx = DC->getASTContext();
  CompletionLookup Lookup(CompletionCtx.getResultSink(), Ctx, DC,
                          &CompletionCtx);

  Lookup.shouldCheckForDuplicates(Results.size() > 1);

  // The type context that is being used for global results.
  ExpectedTypeContext UnifiedTypeContext;
  UnifiedTypeContext.setPreferNonVoid(true);

  for (auto &Result : Results) {
    Lookup.setExpectedTypes({Result.ExpectedTy}, Result.IsImpliedResult,
                            /*expectsNonVoid=*/true);
    Lookup.addPoundAvailable(ParentStmtKind);
    Lookup.addObjCPoundKeywordCompletions(/*needPound=*/false);
    Lookup.getMacroCompletions(CodeCompletionMacroRole::Expression);

    UnifiedTypeContext.merge(*Lookup.getExpectedTypeContext());
  }

  collectCompletionResults(CompletionCtx, Lookup, DC, UnifiedTypeContext,
                           /*CanCurrDeclContextHandleAsync=*/false);
}
