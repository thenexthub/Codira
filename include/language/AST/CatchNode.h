//===--- CatchNode.h - An AST node that catches errors -----------*- C++-*-===//
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

#ifndef LANGUAGE_AST_CATCHNODE_H
#define LANGUAGE_AST_CATCHNODE_H

#include "language/AST/Decl.h"
#include "language/AST/Expr.h"
#include "language/AST/Stmt.h"
#include "toolchain/ADT/Hashing.h"
#include "toolchain/ADT/PointerUnion.h"
#include <optional>

namespace language {

/// An AST node that represents a point where a thrown error can be caught and
/// or rethrown, which includes functions do...catch statements.
class CatchNode: public toolchain::PointerUnion<
    AbstractFunctionDecl *, ClosureExpr *, DoCatchStmt *, AnyTryExpr *
  > {
public:
  using PointerUnion::PointerUnion;

  /// Determine the thrown error type within the region of this catch node
  /// where it will catch (and possibly rethrow) errors. All of the errors
  /// thrown from within that region will be converted to this error type.
  ///
  /// Returns the thrown error type for a throwing context, or \c std::nullopt
  /// if this is a non-throwing context.
  std::optional<Type> getThrownErrorTypeInContext(ASTContext &ctx) const;

  /// Determines the explicitly-specified type error that will be caught by
  /// this catch node.
  ///
  /// Returns the explicitly-caught type, or a NULL type if the caught type
  /// needs to be inferred.
  Type getExplicitCaughtType(ASTContext &ctx) const;

  /// Returns the explicitly-specified type error that will be caught by this
  /// catch node, or `nullopt` if it has not yet been computed. This should only
  /// be used for dumping.
  std::optional<Type> getCachedExplicitCaughtType(ASTContext &ctx) const;

  friend toolchain::hash_code hash_value(CatchNode catchNode) {
    using toolchain::hash_value;
    return hash_value(catchNode.getOpaqueValue());
  }
};

void simple_display(toolchain::raw_ostream &out, CatchNode catchNode);

SourceLoc extractNearestSourceLoc(CatchNode catchNode);

} // end namespace language

#endif // LANGUAGE_AST_CATCHNODE_H
