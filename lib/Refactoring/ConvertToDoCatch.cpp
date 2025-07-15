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
#include "language/Basic/Assertions.h"

using namespace language::refactoring;

static CharSourceRange
findSourceRangeToWrapInCatch(const ResolvedExprStartCursorInfo &CursorInfo,
                             SourceFile *TheFile, SourceManager &SM) {
  Expr *E = CursorInfo.getTrailingExpr();
  if (!E)
    return CharSourceRange();
  auto Node = ASTNode(E);
  auto NodeChecker = [](ASTNode N) { return N.isStmt(StmtKind::Brace); };
  ContextFinder Finder(*TheFile, Node, NodeChecker);
  Finder.resolve();
  auto Contexts = Finder.getContexts();
  if (Contexts.empty())
    return CharSourceRange();
  auto TargetNode = Contexts.back();
  BraceStmt *BStmt = dyn_cast<BraceStmt>(TargetNode.dyn_cast<Stmt *>());
  auto ConvertToCharRange = [&SM](SourceRange SR) {
    return Lexer::getCharSourceRangeFromSourceRange(SM, SR);
  };
  assert(BStmt);
  auto ExprRange = ConvertToCharRange(E->getSourceRange());
  // Check elements of the deepest BraceStmt, pick one that covers expression.
  for (auto Elem : BStmt->getElements()) {
    auto ElemRange = ConvertToCharRange(Elem.getSourceRange());
    if (ElemRange.contains(ExprRange))
      TargetNode = Elem;
  }
  return ConvertToCharRange(TargetNode.getSourceRange());
}

bool RefactoringActionConvertToDoCatch::isApplicable(ResolvedCursorInfoPtr Tok,
                                                     DiagnosticEngine &Diag) {
  auto ExprStartInfo = dyn_cast<ResolvedExprStartCursorInfo>(Tok);
  if (!ExprStartInfo || !ExprStartInfo->getTrailingExpr())
    return false;
  return isa<ForceTryExpr>(ExprStartInfo->getTrailingExpr());
}

bool RefactoringActionConvertToDoCatch::performChange() {
  auto ExprStartInfo = cast<ResolvedExprStartCursorInfo>(CursorInfo);
  auto *TryExpr = dyn_cast<ForceTryExpr>(ExprStartInfo->getTrailingExpr());
  assert(TryExpr);
  auto Range = findSourceRangeToWrapInCatch(*ExprStartInfo, TheFile, SM);
  if (!Range.isValid())
    return true;
  // Wrap given range in do catch block.
  EditConsumer.accept(SM, Range.getStart(), "do {\n");
  EditorConsumerInsertStream OS(EditConsumer, SM, Range.getEnd());
  OS << "\n} catch {\n" << getCodePlaceholder() << "\n}";

  // Delete ! from try! expression
  auto ExclaimLen = getKeywordLen(tok::exclaim_postfix);
  auto ExclaimRange = CharSourceRange(TryExpr->getExclaimLoc(), ExclaimLen);
  EditConsumer.remove(SM, ExclaimRange);
  return false;
}
