//===----------------------------------------------------------------------===//
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

#include "RefactoringActions.h"
#include "language/Basic/Assertions.h"

using namespace language::refactoring;

static std::unique_ptr<toolchain::SetVector<Expr *>>
findConcatenatedExpressions(const ResolvedRangeInfo &Info, ASTContext &Ctx) {
  Expr *E = nullptr;

  switch (Info.Kind) {
  case RangeKind::SingleExpression:
    E = Info.ContainedNodes[0].get<Expr *>();
    break;
  case RangeKind::PartOfExpression:
    E = Info.CommonExprParent;
    break;
  default:
    return nullptr;
  }

  assert(E);

  struct StringInterpolationExprFinder : public SourceEntityWalker {
    std::unique_ptr<toolchain::SetVector<Expr *>> Bucket =
        std::make_unique<toolchain::SetVector<Expr *>>();
    ASTContext &Ctx;

    bool IsValidInterpolation = true;
    StringInterpolationExprFinder(ASTContext &Ctx) : Ctx(Ctx) {}

    bool isConcatenationExpr(DeclRefExpr *Expr) {
      if (!Expr)
        return false;
      auto *FD = dyn_cast<FuncDecl>(Expr->getDecl());
      if (FD == nullptr ||
          (FD != Ctx.getPlusFunctionOnString() &&
           FD != Ctx.getPlusFunctionOnRangeReplaceableCollection())) {
        return false;
      }
      return true;
    }

    bool walkToExprPre(Expr *E) override {
      if (E->isImplicit())
        return true;
      // FIXME: we should have ErrorType instead of null.
      if (E->getType().isNull())
        return true;

      // Only binary concatenation operators should exist in expression
      if (E->getKind() == ExprKind::Binary) {
        auto *BE = dyn_cast<BinaryExpr>(E);
        auto *OperatorDeclRef = BE->getSemanticFn()->getMemberOperatorRef();
        if (!(isConcatenationExpr(OperatorDeclRef) &&
              E->getType()->isString())) {
          IsValidInterpolation = false;
          return false;
        }
        return true;
      }
      // Everything that evaluates to string should be gathered.
      if (E->getType()->isString()) {
        Bucket->insert(E);
        return false;
      }
      if (auto *DR = dyn_cast<DeclRefExpr>(E)) {
        // Checks whether all function references in expression are
        // concatenations.
        auto *FD = dyn_cast<FuncDecl>(DR->getDecl());
        auto IsConcatenation = isConcatenationExpr(DR);
        if (FD && IsConcatenation) {
          return false;
        }
      }
      // There was non-expected expression, it's not valid interpolation then.
      IsValidInterpolation = false;
      return false;
    }
  } Walker(Ctx);
  Walker.walk(E);

  // There should be two or more expressions to convert.
  if (!Walker.IsValidInterpolation || Walker.Bucket->size() < 2)
    return nullptr;

  return std::move(Walker.Bucket);
}

static void interpolatedExpressionForm(Expr *E, SourceManager &SM,
                                       toolchain::raw_ostream &OS) {
  if (auto *Literal = dyn_cast<StringLiteralExpr>(E)) {
    OS << Literal->getValue();
    return;
  }
  auto ExpStr =
      Lexer::getCharSourceRangeFromSourceRange(SM, E->getSourceRange())
          .str()
          .str();
  if (isa<InterpolatedStringLiteralExpr>(E)) {
    ExpStr.erase(0, 1);
    ExpStr.pop_back();
    OS << ExpStr;
    return;
  }
  OS << "\\(" << ExpStr << ")";
}

bool RefactoringActionConvertStringsConcatenationToInterpolation::isApplicable(
    const ResolvedRangeInfo &Info, DiagnosticEngine &Diag) {
  auto RangeContext = Info.RangeContext;
  if (RangeContext) {
    auto &Ctx = Info.RangeContext->getASTContext();
    return findConcatenatedExpressions(Info, Ctx) != nullptr;
  }
  return false;
}

bool RefactoringActionConvertStringsConcatenationToInterpolation::
    performChange() {
  auto Expressions = findConcatenatedExpressions(RangeInfo, Ctx);
  if (!Expressions)
    return true;
  EditorConsumerInsertStream OS(EditConsumer, SM, RangeInfo.ContentRange);
  OS << "\"";
  for (auto It = Expressions->begin(); It != Expressions->end(); ++It) {
    interpolatedExpressionForm(*It, SM, OS);
  }
  OS << "\"";
  return false;
}
