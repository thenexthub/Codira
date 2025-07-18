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

#include "ContextFinder.h"
#include "RefactoringActions.h"
#include "language/AST/Stmt.h"

using namespace language::refactoring;

static CallExpr *findTrailingClosureTarget(SourceManager &SM,
                                           ResolvedCursorInfoPtr CursorInfo) {
  if (CursorInfo->getKind() == CursorInfoKind::StmtStart)
    // StmtStart postion can't be a part of CallExpr.
    return nullptr;

  // Find inner most CallExpr
  ContextFinder Finder(
      *CursorInfo->getSourceFile(), CursorInfo->getLoc(), [](ASTNode N) {
        return N.isStmt(StmtKind::Brace) || N.isExpr(ExprKind::Call);
      });
  Finder.resolve();
  auto contexts = Finder.getContexts();
  if (contexts.empty())
    return nullptr;

  // If the innermost context is a statement (which will be a BraceStmt per
  // the filtering condition above), drop it.
  if (contexts.back().is<Stmt *>()) {
    contexts = contexts.drop_back();
  }

  if (contexts.empty() || !contexts.back().is<Expr *>())
    return nullptr;
  CallExpr *CE = cast<CallExpr>(contexts.back().get<Expr *>());

  // The last argument is a non-trailing closure?
  auto *Args = CE->getArgs();
  if (Args->empty() || Args->hasAnyTrailingClosures())
    return nullptr;

  auto *LastArg = Args->back().getExpr();
  if (auto *ICE = dyn_cast<ImplicitConversionExpr>(LastArg))
    LastArg = ICE->getSyntacticSubExpr();

  if (isa<ClosureExpr>(LastArg) || isa<CaptureListExpr>(LastArg))
    return CE;
  return nullptr;
}

bool RefactoringActionTrailingClosure::isApplicable(
    ResolvedCursorInfoPtr CursorInfo, DiagnosticEngine &Diag) {
  SourceManager &SM = CursorInfo->getSourceFile()->getASTContext().SourceMgr;
  return findTrailingClosureTarget(SM, CursorInfo);
}

bool RefactoringActionTrailingClosure::performChange() {
  auto *CE = findTrailingClosureTarget(SM, CursorInfo);
  if (!CE)
    return true;

  auto *ArgList = CE->getArgs()->getOriginalArgs();
  auto LParenLoc = ArgList->getLParenLoc();
  auto RParenLoc = ArgList->getRParenLoc();

  if (LParenLoc.isInvalid() || RParenLoc.isInvalid())
    return true;

  auto NumArgs = ArgList->size();
  if (NumArgs == 0)
    return true;

  auto *ClosureArg = ArgList->getExpr(NumArgs - 1);
  if (auto *ICE = dyn_cast<ImplicitConversionExpr>(ClosureArg))
    ClosureArg = ICE->getSyntacticSubExpr();

  // Replace:
  //   * Open paren with ' ' if the closure is sole argument.
  //   * Comma with ') ' otherwise.
  if (NumArgs > 1) {
    auto *PrevArg = ArgList->getExpr(NumArgs - 2);
    CharSourceRange PreRange(
        SM, Lexer::getLocForEndOfToken(SM, PrevArg->getEndLoc()),
        ClosureArg->getStartLoc());
    EditConsumer.accept(SM, PreRange, ") ");
  } else {
    CharSourceRange PreRange(SM, LParenLoc, ClosureArg->getStartLoc());
    EditConsumer.accept(SM, PreRange, " ");
  }
  // Remove original closing paren.
  CharSourceRange PostRange(
      SM, Lexer::getLocForEndOfToken(SM, ClosureArg->getEndLoc()),
      Lexer::getLocForEndOfToken(SM, RParenLoc));
  EditConsumer.remove(SM, PostRange);
  return false;
}
