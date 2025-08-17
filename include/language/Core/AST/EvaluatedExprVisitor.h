//===--- EvaluatedExprVisitor.h - Evaluated expression visitor --*- C++ -*-===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
// 
// Author: Tunjay Akbarli
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// 
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//
//
//  This file defines the EvaluatedExprVisitor class template, which visits
//  the potentially-evaluated subexpressions of a potentially-evaluated
//  expression.
//
//===----------------------------------------------------------------------===//
#ifndef LANGUAGE_CORE_AST_EVALUATEDEXPRVISITOR_H
#define LANGUAGE_CORE_AST_EVALUATEDEXPRVISITOR_H

#include "language/Core/AST/DeclCXX.h"
#include "language/Core/AST/Expr.h"
#include "language/Core/AST/ExprCXX.h"
#include "language/Core/AST/StmtVisitor.h"
#include "toolchain/ADT/STLExtras.h"

namespace language::Core {

class ASTContext;

/// Given a potentially-evaluated expression, this visitor visits all
/// of its potentially-evaluated subexpressions, recursively.
template<template <typename> class Ptr, typename ImplClass>
class EvaluatedExprVisitorBase : public StmtVisitorBase<Ptr, ImplClass, void> {
protected:
  const ASTContext &Context;

public:
  // Return whether this visitor should recurse into discarded statements for a
  // 'constexpr-if'.
  bool shouldVisitDiscardedStmt() const { return true; }
#define PTR(CLASS) typename Ptr<CLASS>::type

  explicit EvaluatedExprVisitorBase(const ASTContext &Context) : Context(Context) { }

  // Expressions that have no potentially-evaluated subexpressions (but may have
  // other sub-expressions).
  void VisitDeclRefExpr(PTR(DeclRefExpr) E) { }
  void VisitOffsetOfExpr(PTR(OffsetOfExpr) E) { }
  void VisitUnaryExprOrTypeTraitExpr(PTR(UnaryExprOrTypeTraitExpr) E) { }
  void VisitExpressionTraitExpr(PTR(ExpressionTraitExpr) E) { }
  void VisitBlockExpr(PTR(BlockExpr) E) { }
  void VisitCXXUuidofExpr(PTR(CXXUuidofExpr) E) { }
  void VisitCXXNoexceptExpr(PTR(CXXNoexceptExpr) E) { }

  void VisitMemberExpr(PTR(MemberExpr) E) {
    // Only the base matters.
    return this->Visit(E->getBase());
  }

  void VisitChooseExpr(PTR(ChooseExpr) E) {
    // Don't visit either child expression if the condition is dependent.
    if (E->getCond()->isValueDependent())
      return;
    // Only the selected subexpression matters; the other one is not evaluated.
    return this->Visit(E->getChosenSubExpr());
  }

  void VisitGenericSelectionExpr(PTR(GenericSelectionExpr) E) {
    // The controlling expression of a generic selection is not evaluated.

    // Don't visit either child expression if the condition is type-dependent.
    if (E->isResultDependent())
      return;
    // Only the selected subexpression matters; the other subexpressions and the
    // controlling expression are not evaluated.
    return this->Visit(E->getResultExpr());
  }

  void VisitDesignatedInitExpr(PTR(DesignatedInitExpr) E) {
    // Only the actual initializer matters; the designators are all constant
    // expressions.
    return this->Visit(E->getInit());
  }

  void VisitCXXTypeidExpr(PTR(CXXTypeidExpr) E) {
    if (E->isPotentiallyEvaluated())
      return this->Visit(E->getExprOperand());
  }

  void VisitCallExpr(PTR(CallExpr) CE) {
    if (!CE->isUnevaluatedBuiltinCall(Context))
      return getDerived().VisitExpr(CE);
  }

  void VisitLambdaExpr(PTR(LambdaExpr) LE) {
    // Only visit the capture initializers, and not the body.
    for (LambdaExpr::const_capture_init_iterator I = LE->capture_init_begin(),
                                                 E = LE->capture_init_end();
         I != E; ++I)
      if (*I)
        this->Visit(*I);
  }

  /// The basis case walks all of the children of the statement or
  /// expression, assuming they are all potentially evaluated.
  void VisitStmt(PTR(Stmt) S) {
    for (auto *SubStmt : S->children())
      if (SubStmt)
        this->Visit(SubStmt);
  }

  void VisitIfStmt(PTR(IfStmt) If) {
    if (!getDerived().shouldVisitDiscardedStmt()) {
      if (auto SubStmt = If->getNondiscardedCase(Context)) {
        if (*SubStmt)
          this->Visit(*SubStmt);
        return;
      }
    }

    getDerived().VisitStmt(If);
  }

  ImplClass &getDerived() { return *static_cast<ImplClass *>(this); }

#undef PTR
};

/// EvaluatedExprVisitor - This class visits 'Expr *'s
template <typename ImplClass>
class EvaluatedExprVisitor
    : public EvaluatedExprVisitorBase<std::add_pointer, ImplClass> {
public:
  explicit EvaluatedExprVisitor(const ASTContext &Context)
      : EvaluatedExprVisitorBase<std::add_pointer, ImplClass>(Context) {}
};

/// ConstEvaluatedExprVisitor - This class visits 'const Expr *'s.
template <typename ImplClass>
class ConstEvaluatedExprVisitor
    : public EvaluatedExprVisitorBase<toolchain::make_const_ptr, ImplClass> {
public:
  explicit ConstEvaluatedExprVisitor(const ASTContext &Context)
      : EvaluatedExprVisitorBase<toolchain::make_const_ptr, ImplClass>(Context) {}
};
}

#endif // LANGUAGE_CORE_AST_EVALUATEDEXPRVISITOR_H
