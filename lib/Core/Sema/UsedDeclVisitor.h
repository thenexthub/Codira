//===- UsedDeclVisitor.h - ODR-used declarations visitor --------*- C++ -*-===//
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
//===----------------------------------------------------------------------===//
//
//  This file defines UsedDeclVisitor, a CRTP class which visits all the
//  declarations that are ODR-used by an expression or statement.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_LIB_SEMA_USEDDECLVISITOR_H
#define LANGUAGE_CORE_LIB_SEMA_USEDDECLVISITOR_H

#include "language/Core/AST/EvaluatedExprVisitor.h"
#include "language/Core/Sema/SemaInternal.h"

namespace language::Core {
template <class Derived>
class UsedDeclVisitor : public EvaluatedExprVisitor<Derived> {
protected:
  Sema &S;

public:
  typedef EvaluatedExprVisitor<Derived> Inherited;

  UsedDeclVisitor(Sema &S) : Inherited(S.Context), S(S) {}

  Derived &asImpl() { return *static_cast<Derived *>(this); }

  void VisitDeclRefExpr(DeclRefExpr *E) {
    auto *D = E->getDecl();
    if (isa<FunctionDecl>(D) || isa<VarDecl>(D)) {
      asImpl().visitUsedDecl(E->getLocation(), D);
    }
  }

  void VisitMemberExpr(MemberExpr *E) {
    auto *D = E->getMemberDecl();
    if (isa<FunctionDecl>(D) || isa<VarDecl>(D)) {
      asImpl().visitUsedDecl(E->getMemberLoc(), D);
    }
    asImpl().Visit(E->getBase());
  }

  void VisitCapturedStmt(CapturedStmt *Node) {
    asImpl().visitUsedDecl(Node->getBeginLoc(), Node->getCapturedDecl());
    Inherited::VisitCapturedStmt(Node);
  }

  void VisitCXXBindTemporaryExpr(CXXBindTemporaryExpr *E) {
    asImpl().visitUsedDecl(
        E->getBeginLoc(),
        const_cast<CXXDestructorDecl *>(E->getTemporary()->getDestructor()));
    asImpl().Visit(E->getSubExpr());
  }

  void VisitCXXNewExpr(CXXNewExpr *E) {
    if (E->getOperatorNew())
      asImpl().visitUsedDecl(E->getBeginLoc(), E->getOperatorNew());
    if (E->getOperatorDelete())
      asImpl().visitUsedDecl(E->getBeginLoc(), E->getOperatorDelete());
    Inherited::VisitCXXNewExpr(E);
  }

  void VisitCXXDeleteExpr(CXXDeleteExpr *E) {
    if (E->getOperatorDelete())
      asImpl().visitUsedDecl(E->getBeginLoc(), E->getOperatorDelete());
    QualType DestroyedOrNull = E->getDestroyedType();
    if (!DestroyedOrNull.isNull()) {
      QualType Destroyed = S.Context.getBaseElementType(DestroyedOrNull);
      if (const RecordType *DestroyedRec = Destroyed->getAs<RecordType>()) {
        CXXRecordDecl *Record =
            cast<CXXRecordDecl>(DestroyedRec->getOriginalDecl());
        if (auto *Def = Record->getDefinition())
          asImpl().visitUsedDecl(E->getBeginLoc(), S.LookupDestructor(Def));
      }
    }

    Inherited::VisitCXXDeleteExpr(E);
  }

  void VisitCXXConstructExpr(CXXConstructExpr *E) {
    asImpl().visitUsedDecl(E->getBeginLoc(), E->getConstructor());
    CXXConstructorDecl *D = E->getConstructor();
    for (const CXXCtorInitializer *Init : D->inits()) {
      if (Init->isInClassMemberInitializer())
        asImpl().Visit(Init->getInit());
    }
    Inherited::VisitCXXConstructExpr(E);
  }

  void VisitCXXDefaultArgExpr(CXXDefaultArgExpr *E) {
    asImpl().Visit(E->getExpr());
    Inherited::VisitCXXDefaultArgExpr(E);
  }

  void VisitCXXDefaultInitExpr(CXXDefaultInitExpr *E) {
    asImpl().Visit(E->getExpr());
    Inherited::VisitCXXDefaultInitExpr(E);
  }

  void VisitInitListExpr(InitListExpr *ILE) {
    if (ILE->hasArrayFiller())
      asImpl().Visit(ILE->getArrayFiller());
    Inherited::VisitInitListExpr(ILE);
  }

  void visitUsedDecl(SourceLocation Loc, Decl *D) {
    if (auto *CD = dyn_cast<CapturedDecl>(D)) {
      if (auto *S = CD->getBody()) {
        asImpl().Visit(S);
      }
    } else if (auto *CD = dyn_cast<BlockDecl>(D)) {
      if (auto *S = CD->getBody()) {
        asImpl().Visit(S);
      }
    }
  }
};
} // end namespace language::Core

#endif // LANGUAGE_CORE_LIB_SEMA_USEDDECLVISITOR_H
