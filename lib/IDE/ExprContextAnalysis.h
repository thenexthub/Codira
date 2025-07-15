//===--- ExprContextAnalysis.h - Expession context analysis ---------------===//
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

#ifndef LANGUAGE_IDE_EXPRCONTEXTANALYSIS_H
#define LANGUAGE_IDE_EXPRCONTEXTANALYSIS_H

#include "language/AST/Type.h"
#include "language/AST/TypeCheckRequests.h"
#include "language/AST/Types.h"
#include "language/Basic/Toolchain.h"
#include "language/Basic/SourceLoc.h"
#include "language/IDE/PossibleParamInfo.h"

namespace language {
class DeclContext;
class Expr;
class ValueDecl;

namespace ide {
enum class SemanticContextKind : uint8_t;

/// Type check parent contexts of the given decl context, and the body of the
/// given context until \c Loc if the context is a function body.
void typeCheckContextAt(TypeCheckASTNodeAtLocContext TypeCheckCtx,
                        SourceLoc Loc);

/// From \p DC, find and returns the outer most expression which source range is
/// exact the same as \p TargetRange. Returns \c nullptr if not found.
Expr *findParsedExpr(const DeclContext *DC, SourceRange TargetRange);

/// Collects possible expected return types of the given decl context.
/// \p DC should be an \c AbstractFunctionDecl or an \c AbstractClosureExpr.
void collectPossibleReturnTypesFromContext(DeclContext *DC,
                                           SmallVectorImpl<Type> &candidates);

struct FunctionTypeAndDecl {
  AnyFunctionType *Type;
  ValueDecl *Decl;
  std::optional<SemanticContextKind> SemanticContext;

  FunctionTypeAndDecl(AnyFunctionType *Type, ValueDecl *Decl)
      : Type(Type), Decl(Decl) {}
  FunctionTypeAndDecl(AnyFunctionType *Type, ValueDecl *Decl,
                      SemanticContextKind SemanticContext)
      : Type(Type), Decl(Decl), SemanticContext(SemanticContext) {}
};

} // namespace ide
} // namespace language

#endif // LANGUAGE_IDE_EXPRCONTEXTANALYSIS_H
