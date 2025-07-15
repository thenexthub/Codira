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
#include "language/AST/Expr.h"
#include "language/AST/Stmt.h"

using namespace language::refactoring;

bool RefactoringActionConvertIfLetExprToGuardExpr::isApplicable(
    const ResolvedRangeInfo &Info, DiagnosticEngine &Diag) {

  if (Info.Kind != RangeKind::SingleStatement &&
      Info.Kind != RangeKind::MultiStatement)
    return false;

  if (Info.ContainedNodes.empty())
    return false;

  IfStmt *If = nullptr;

  if (Info.ContainedNodes.size() == 1) {
    if (auto S = Info.ContainedNodes[0].dyn_cast<Stmt *>()) {
      If = dyn_cast<IfStmt>(S);
    }
  }

  if (!If)
    return false;

  auto CondList = If->getCond();

  if (CondList.size() == 1) {
    auto E = CondList[0];
    auto P = E.getKind();
    if (P == language::StmtConditionElement::CK_PatternBinding) {
      auto Body = dyn_cast_or_null<BraceStmt>(If->getThenStmt());
      if (Body)
        return true;
    }
  }

  return false;
}

bool RefactoringActionConvertIfLetExprToGuardExpr::performChange() {

  auto S = RangeInfo.ContainedNodes[0].dyn_cast<Stmt *>();
  IfStmt *If = dyn_cast<IfStmt>(S);
  auto CondList = If->getCond();

  // Get if-let condition
  SourceRange range = CondList[0].getSourceRange();
  SourceManager &SM = RangeInfo.RangeContext->getASTContext().SourceMgr;
  auto CondCharRange = Lexer::getCharSourceRangeFromSourceRange(SM, range);

  auto Body = dyn_cast_or_null<BraceStmt>(If->getThenStmt());

  // Get if-let then body.
  auto firstElement = Body->getFirstElement();
  auto lastElement = Body->getLastElement();
  SourceRange bodyRange = firstElement.getSourceRange();
  bodyRange.widen(lastElement.getSourceRange());
  auto BodyCharRange = Lexer::getCharSourceRangeFromSourceRange(SM, bodyRange);

  SmallString<64> DeclBuffer;
  toolchain::raw_svector_ostream OS(DeclBuffer);

  StringRef Space = " ";
  StringRef NewLine = "\n";

  OS << tok::kw_guard << Space;
  OS << CondCharRange.str().str() << Space;
  OS << tok::kw_else << Space;
  OS << tok::l_brace << NewLine;

  // Get if-let else body.
  if (auto *ElseBody = dyn_cast_or_null<BraceStmt>(If->getElseStmt())) {
    auto firstElseElement = ElseBody->getFirstElement();
    auto lastElseElement = ElseBody->getLastElement();
    SourceRange elseBodyRange = firstElseElement.getSourceRange();
    elseBodyRange.widen(lastElseElement.getSourceRange());
    auto ElseBodyCharRange =
        Lexer::getCharSourceRangeFromSourceRange(SM, elseBodyRange);
    OS << ElseBodyCharRange.str().str() << NewLine;
  }

  OS << tok::kw_return << NewLine;
  OS << tok::r_brace << NewLine;
  OS << BodyCharRange.str().str();

  // Replace if-let to guard
  auto ReplaceRange = RangeInfo.ContentRange;
  EditConsumer.accept(SM, ReplaceRange, DeclBuffer.str());

  return false;
}
