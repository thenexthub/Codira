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

#include "AsyncRefactoring.h"
#include "Utils.h"

using namespace language;
using namespace language::refactoring::asyncrefactorings;

void DeclCollector::collect(BraceStmt *Scope, SourceFile &SF,
                            toolchain::DenseSet<const Decl *> &Decls) {
  DeclCollector Collector(Decls);
  if (Scope) {
    for (auto Node : Scope->getElements()) {
      Collector.walk(Node);
    }
  } else {
    Collector.walk(SF);
  }
}

bool DeclCollector::walkToDeclPre(Decl *D, CharSourceRange Range) {
  // Want to walk through top level code decls (which are implicitly added
  // for top level non-decl code) and pattern binding decls (which contain
  // the var decls that we care about).
  if (isa<TopLevelCodeDecl>(D) || isa<PatternBindingDecl>(D))
    return true;

  if (!D->isImplicit())
    Decls.insert(D);
  return false;
}

bool DeclCollector::walkToExprPre(Expr *E) { return !isa<ClosureExpr>(E); }

bool DeclCollector::walkToStmtPre(Stmt *S) {
  return S->isImplicit() || !startsNewScope(S);
}
