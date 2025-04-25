//===--- BorrowScopeOpts.cpp ----------------------------------------------===//
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
///
/// \file
///
/// Optimizations that attempt to simplify and or eliminate borrow scopes. Today
/// we only eliminate scopes, but we could also eliminate redundant scopes by
/// converting struct_extract operations to use destructure operations.
///
//===----------------------------------------------------------------------===//

#include "Context.h"
#include "SemanticARCOptVisitor.h"

using namespace language;
using namespace language::semanticarc;

bool SemanticARCOptVisitor::visitBeginBorrowInst(BeginBorrowInst *bbi) {
  // Quickly check if we are supposed to perform this transformation.
  if (!ctx.shouldPerform(ARCTransformKind::RedundantBorrowScopeElimPeephole))
    return false;

  // Non-redundant, lexical borrow scopes must remain in order to ensure that
  // value lifetimes are not observably shortened.
  if (bbi->isLexical() && !isNestedLexicalBeginBorrow(bbi)) {
    return false;
  }

  auto kind = bbi->getOperand()->getOwnershipKind();
  SmallVector<EndBorrowInst *, 16> endBorrows;
  for (auto *op : bbi->getUses()) {
    if (!op->isLifetimeEnding()) {
      // Make sure that this operand can accept our arguments kind.
      if (op->canAcceptKind(kind))
        continue;
      return false;
    }

    // Otherwise, this borrow is being consumed. See if our consuming inst is an
    // end_borrow. If it isn't, then return false, this scope is
    // needed. Otherwise, add the end_borrow to our list of end borrows.
    auto *ebi = dyn_cast<EndBorrowInst>(op->getUser());
    if (!ebi) {
      return false;
    }
    endBorrows.push_back(ebi);
  }

  // At this point, we know that the begin_borrow's operand can be
  // used as an argument to all non-end borrow uses. Eliminate the
  // begin borrow and end borrows.
  while (!endBorrows.empty()) {
    auto *ebi = endBorrows.pop_back_val();
    eraseInstruction(ebi);
  }

  eraseAndRAUWSingleValueInstruction(bbi, bbi->getOperand());
  return true;
}
