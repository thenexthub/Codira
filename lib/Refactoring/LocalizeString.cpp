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

using namespace language::refactoring;

static Expr *findLocalizeTarget(ResolvedCursorInfoPtr CursorInfo) {
  auto ExprStartInfo = dyn_cast<ResolvedExprStartCursorInfo>(CursorInfo);
  if (!ExprStartInfo)
    return nullptr;
  struct StringLiteralFinder : public SourceEntityWalker {
    SourceLoc StartLoc;
    Expr *Target;
    StringLiteralFinder(SourceLoc StartLoc)
        : StartLoc(StartLoc), Target(nullptr) {}
    bool walkToExprPre(Expr *E) override {
      if (E->getStartLoc() != StartLoc)
        return false;
      if (E->getKind() == ExprKind::InterpolatedStringLiteral)
        return false;
      if (E->getKind() == ExprKind::StringLiteral) {
        Target = E;
        return false;
      }
      return true;
    }
  } Walker(ExprStartInfo->getTrailingExpr()->getStartLoc());
  Walker.walk(ExprStartInfo->getTrailingExpr());
  return Walker.Target;
}

bool RefactoringActionLocalizeString::isApplicable(ResolvedCursorInfoPtr Tok,
                                                   DiagnosticEngine &Diag) {
  return findLocalizeTarget(Tok);
}

bool RefactoringActionLocalizeString::performChange() {
  Expr *Target = findLocalizeTarget(CursorInfo);
  if (!Target)
    return true;
  EditConsumer.accept(SM, Target->getStartLoc(), "NSLocalizedString(");
  EditConsumer.insertAfter(SM, Target->getEndLoc(), ", comment: \"\")");
  return false;
}
