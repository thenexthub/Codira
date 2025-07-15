//===--- IfConfigClause.h ---------------------------------------*- C++ -*-===//
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
//
// This file defines the IfConfigClause.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_AST_IFCONFIGCLAUSE_H
#define LANGUAGE_AST_IFCONFIGCLAUSE_H

#include "toolchain/ADT/ArrayRef.h"

namespace language {
  class Expr;
  class SourceLoc;
  struct ASTNode;

/// This represents one part of a #if block.  If the condition field is
/// non-null, then this represents a #if or a #elseif, otherwise it represents
/// an #else block.
struct IfConfigClause {
  /// The location of the #if, #elseif, or #else keyword.
  SourceLoc Loc;
  
  /// The condition guarding this #if or #elseif block.  If this is null, this
  /// is a #else clause.
  Expr *Cond;
  
  /// Elements inside the clause
  ArrayRef<ASTNode> Elements;

  /// True if this is the active clause of the #if block.
  const bool isActive;

  IfConfigClause(SourceLoc Loc, Expr *Cond,
                 ArrayRef<ASTNode> Elements, bool isActive)
    : Loc(Loc), Cond(Cond), Elements(Elements), isActive(isActive) {
  }
};

} // end namespace language

#endif
