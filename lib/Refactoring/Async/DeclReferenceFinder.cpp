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

using namespace language;
using namespace language::refactoring::asyncrefactorings;

bool DeclReferenceFinder::walkToExprPre(Expr *E) {
  if (auto DRE = dyn_cast<DeclRefExpr>(E)) {
    if (DRE->getDecl() == Search) {
      HasFoundReference = true;
      return false;
    }
  }
  return true;
}

bool DeclReferenceFinder::containsReference(ASTNode Node,
                                            const ValueDecl *Search) {
  DeclReferenceFinder Checker(Search);
  Checker.walk(Node);
  return Checker.HasFoundReference;
}
