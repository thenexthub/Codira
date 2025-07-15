//===--- UnresolvedMemberCodeCompletion.cpp -------------------------------===//
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

#include "language/Basic/Assertions.h"
#include "language/IDE/UnresolvedMemberCompletion.h"
#include "language/IDE/CodeCompletion.h"
#include "language/IDE/CompletionLookup.h"
#include "language/Sema/CompletionContextFinder.h"
#include "language/Sema/ConstraintSystem.h"
#include "language/Sema/IDETypeChecking.h"

using namespace language;
using namespace language::constraints;
using namespace language::ide;

bool UnresolvedMemberTypeCheckCompletionCallback::Result::tryMerge(
    const Result &Other, DeclContext *DC) {
  auto expectedTy = tryMergeBaseTypeForCompletionLookup(ExpectedTy,
                                                        Other.ExpectedTy, DC);
  if (!expectedTy)
    return false;

  ExpectedTy = expectedTy;

  IsImpliedResult |= Other.IsImpliedResult;
  IsInAsyncContext |= Other.IsInAsyncContext;
  return true;
}

void UnresolvedMemberTypeCheckCompletionCallback::addExprResult(
    const Result &Res) {
  for (auto idx : indices(ExprResults)) {
    if (ExprResults[idx].tryMerge(Res, DC))
      return;
  }
  ExprResults.push_back(Res);
}

void UnresolvedMemberTypeCheckCompletionCallback::sawSolutionImpl(
    const constraints::Solution &S) {
  Type ExpectedTy = getTypeForCompletion(S, CompletionExpr);
  bool IsAsync = isContextAsync(S, DC);

  // If the type couldn't be determined (e.g. because there isn't any context
  // to derive it from), let's not attempt to do a lookup since it wouldn't
  // produce any useful results anyway.
  if (ExpectedTy) {
    bool IsImpliedResult = isImpliedResult(S, CompletionExpr);
    Result Res = {ExpectedTy, IsImpliedResult, IsAsync};
    addExprResult(Res);
  }

  if (auto PatternType = getPatternMatchType(S, CompletionExpr)) {
    auto IsEqual = [&](const Result &R) {
      return R.ExpectedTy->isEqual(PatternType);
    };
    if (!toolchain::any_of(EnumPatternTypes, IsEqual)) {
      EnumPatternTypes.push_back(
          {PatternType, /*isImpliedResult=*/false, IsAsync});
    }
  }
}

void UnresolvedMemberTypeCheckCompletionCallback::collectResults(
    DeclContext *DC, SourceLoc DotLoc,
    ide::CodeCompletionContext &CompletionCtx) {
  ASTContext &Ctx = DC->getASTContext();
  CompletionLookup Lookup(CompletionCtx.getResultSink(), Ctx, DC,
                          &CompletionCtx);

  assert(DotLoc.isValid());
  Lookup.setHaveDot(DotLoc);
  Lookup.shouldCheckForDuplicates(ExprResults.size() + EnumPatternTypes.size() >
                                  1);

  // Get the canonical versions of the top-level types
  SmallPtrSet<CanType, 4> originalTypes;
  for (auto &Result : ExprResults)
    originalTypes.insert(Result.ExpectedTy->getCanonicalType());

  for (auto &Result : ExprResults) {
    Lookup.setExpectedTypes({Result.ExpectedTy}, Result.IsImpliedResult,
                            /*expectsNonVoid*/ true);
    Lookup.setIdealExpectedType(Result.ExpectedTy);
    Lookup.setCanCurrDeclContextHandleAsync(Result.IsInAsyncContext);

    // For optional types, also get members of the unwrapped type if it's not
    // already equivalent to one of the top-level types. Handling it via the top
    // level type and not here ensures we give the correct type relation
    // (identical, rather than convertible).
    if (Result.ExpectedTy->getOptionalObjectType()) {
      Type Unwrapped = Result.ExpectedTy->lookThroughAllOptionalTypes();
      if (originalTypes.insert(Unwrapped->getCanonicalType()).second)
        Lookup.getUnresolvedMemberCompletions(Unwrapped);
    }
    Lookup.getUnresolvedMemberCompletions(Result.ExpectedTy);
  }

  // The type context that is being used for global results.
  ExpectedTypeContext UnifiedTypeContext;
  UnifiedTypeContext.setPreferNonVoid(true);
  bool UnifiedCanHandleAsync = false;

  // Offer completions when interpreting the pattern match as an
  // EnumElementPattern.
  for (auto &Result : EnumPatternTypes) {
    Type Ty = Result.ExpectedTy;
    Lookup.setExpectedTypes({Ty}, /*isImpliedResult=*/false,
                            /*expectsNonVoid=*/true);
    Lookup.setIdealExpectedType(Ty);
    Lookup.setCanCurrDeclContextHandleAsync(Result.IsInAsyncContext);

    // We can pattern match MyEnum against Optional<MyEnum>
    if (Ty->getOptionalObjectType()) {
      Type Unwrapped = Ty->lookThroughAllOptionalTypes();
      Lookup.getEnumElementPatternCompletions(Unwrapped);
    }

    Lookup.getEnumElementPatternCompletions(Ty);

    UnifiedTypeContext.merge(*Lookup.getExpectedTypeContext());
    UnifiedCanHandleAsync |= Result.IsInAsyncContext;
  }

  collectCompletionResults(CompletionCtx, Lookup, DC, UnifiedTypeContext,
                           UnifiedCanHandleAsync);
}
