//===--- ExprContextAnalysis.h - Expession context analysis ---------------===//
//
// This source file is part of the Swift.org open source project
//
// Copyright (c) 2019 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See https://swift.org/LICENSE.txt for license information
// See https://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
//
//===----------------------------------------------------------------------===//

#ifndef SWIFT_IDE_EXPRCONTEXTANALYSIS_H
#define SWIFT_IDE_EXPRCONTEXTANALYSIS_H

#include "language/AST/Type.h"
#include "language/AST/TypeCheckRequests.h"
#include "language/AST/Types.h"
#include "language/Basic/LLVM.h"
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

#endif // SWIFT_IDE_EXPRCONTEXTANALYSIS_H
