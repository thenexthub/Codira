//===--- ASTVisitor.h - SILGen ASTVisitor specialization --------*- C++ -*-===//
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
// This file defines language::Lowering::ASTVisitor.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_LOWERING_ASTVISITOR_H
#define LANGUAGE_LOWERING_ASTVISITOR_H

#include "language/AST/ASTVisitor.h"

namespace language {
namespace Lowering {
  
/// Lowering::ASTVisitor - This is a specialization of
/// language::ASTVisitor which works only on resolved nodes and
/// which automatically ignores certain AST node kinds.
template<typename ImplClass,
         typename ExprRetTy = void,
         typename StmtRetTy = void,
         typename DeclRetTy = void,
         typename PatternRetTy = void,
         typename... Args>
class ASTVisitor : public language::ASTVisitor<ImplClass,
                                            ExprRetTy,
                                            StmtRetTy,
                                            DeclRetTy,
                                            PatternRetTy,
                                            void,
                                            void,
                                            Args...>
{
public:
#define EXPR(Id, Parent)
#define UNCHECKED_EXPR(Id, Parent) \
  ExprRetTy visit##Id##Expr(Id##Expr *E, Args...AA) { \
    toolchain_unreachable(#Id "Expr should not survive to SILGen"); \
  }
#include "language/AST/ExprNodes.def"
  
  ExprRetTy visitErrorExpr(ErrorExpr *E, Args...AA) {
    toolchain_unreachable("expression kind should not survive to SILGen");
  }

  ExprRetTy visitCodeCompletionExpr(CodeCompletionExpr *E, Args...AA) {
    toolchain_unreachable("expression kind should not survive to SILGen");
  }

  ExprRetTy visitDefaultArgumentExpr(DefaultArgumentExpr *E, Args...AA) {
    toolchain_unreachable("DefaultArgumentExpr should not appear in this position");
  }

  ExprRetTy visitVarargExpansionExpr(VarargExpansionExpr *E, Args... AA) {
    return static_cast<ImplClass*>(this)->visit(E->getSubExpr(),
                                                std::forward<Args>(AA)...);
  }

  ExprRetTy visitIdentityExpr(IdentityExpr *E, Args...AA) {
    return static_cast<ImplClass*>(this)->visit(E->getSubExpr(),
                                                std::forward<Args>(AA)...);
  }

  ExprRetTy visitTryExpr(TryExpr *E, Args...AA) {
    return static_cast<ImplClass*>(this)->visit(E->getSubExpr(),
                                                std::forward<Args>(AA)...);
  }

  ExprRetTy visitLazyInitializerExpr(LazyInitializerExpr *E, Args...AA) {
    return static_cast<ImplClass*>(this)->visit(E->getSubExpr(),
                                                std::forward<Args>(AA)...);
  }
};

template <typename ImplClass,
          typename ExprRetTy = void,
          typename... Args>
using ExprVisitor = ASTVisitor<ImplClass, ExprRetTy, void, void, void, Args...>;

} // end namespace Lowering
} // end namespace language

#endif
