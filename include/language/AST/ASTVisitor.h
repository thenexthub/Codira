//===--- ASTVisitor.h - Decl, Expr and Stmt Visitor -------------*- C++ -*-===//
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
// This file defines the ASTVisitor class, and the DeclVisitor, ExprVisitor, and
// StmtVisitor template typedefs.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_AST_ASTVISITOR_H
#define LANGUAGE_AST_ASTVISITOR_H

#include "language/AST/Decl.h"
#include "language/AST/Expr.h"
#include "language/AST/Module.h"
#include "language/AST/Pattern.h"
#include "language/AST/Stmt.h"
#include "language/AST/TypeRepr.h"
#include "toolchain/Support/ErrorHandling.h"

namespace language {
  class ParameterList;
  
/// ASTVisitor - This is a simple visitor class for Codira expressions.
template<typename ImplClass,
         typename ExprRetTy = void,
         typename StmtRetTy = void,
         typename DeclRetTy = void,
         typename PatternRetTy = void,
         typename TypeReprRetTy = void,
         typename AttributeRetTy = void,
         typename... Args>
class ASTVisitor {
public:
  typedef ASTVisitor ASTVisitorType;

  DeclRetTy visit(Decl *D, Args... AA) {
    switch (D->getKind()) {
#define DECL(CLASS, PARENT) \
    case DeclKind::CLASS: \
      return static_cast<ImplClass*>(this) \
        ->visit##CLASS##Decl(static_cast<CLASS##Decl*>(D), \
                             ::std::forward<Args>(AA)...);
#include "language/AST/DeclNodes.def"
    }
    toolchain_unreachable("Not reachable, all cases handled");
  }

  ExprRetTy visit(Expr *E, Args... AA) {
    switch (E->getKind()) {

#define EXPR(CLASS, PARENT) \
    case ExprKind::CLASS: \
      return static_cast<ImplClass*>(this) \
        ->visit##CLASS##Expr(static_cast<CLASS##Expr*>(E), \
                             ::std::forward<Args>(AA)...);
#include "language/AST/ExprNodes.def"

    }
    toolchain_unreachable("Not reachable, all cases handled");
  }
  
  // Provide default implementations of abstract "visit" implementations that
  // just chain to their base class.  This allows visitors to just implement
  // the base behavior and handle all subclasses if they desire.  Since this is
  // a template, it will only instantiate cases that are used and thus we still
  // require full coverage of the AST nodes by the visitor.
#define ABSTRACT_EXPR(CLASS, PARENT)                                \
  ExprRetTy visit##CLASS##Expr(CLASS##Expr *E, Args... AA) {  \
     return static_cast<ImplClass*>(this)->visit##PARENT(E, \
                                                ::std::forward<Args>(AA)...);  \
  }
#define EXPR(CLASS, PARENT) ABSTRACT_EXPR(CLASS, PARENT)
#include "language/AST/ExprNodes.def"

  StmtRetTy visit(Stmt *S, Args... AA) {
    switch (S->getKind()) {

#define STMT(CLASS, PARENT) \
    case StmtKind::CLASS: \
      return static_cast<ImplClass*>(this) \
        ->visit##CLASS##Stmt(static_cast<CLASS##Stmt*>(S), \
                             ::std::forward<Args>(AA)...);
#include "language/AST/StmtNodes.def"

    }
    toolchain_unreachable("Not reachable, all cases handled");
  }

#define DECL(CLASS, PARENT) \
  DeclRetTy visit##CLASS##Decl(CLASS##Decl *D, Args... AA) {\
    return static_cast<ImplClass*>(this)->visit##PARENT(D, \
                                                 ::std::forward<Args>(AA)...); \
  }
#define ABSTRACT_DECL(CLASS, PARENT) DECL(CLASS, PARENT)
#include "language/AST/DeclNodes.def"

  PatternRetTy visit(Pattern *P, Args... AA) {
    switch (P->getKind()) {
#define PATTERN(CLASS, PARENT) \
    case PatternKind::CLASS: \
      return static_cast<ImplClass*>(this) \
        ->visit##CLASS##Pattern(static_cast<CLASS##Pattern*>(P), \
                                ::std::forward<Args>(AA)...);
#include "language/AST/PatternNodes.def"
    }
    toolchain_unreachable("Not reachable, all cases handled");
  }

  TypeReprRetTy visit(TypeRepr *T, Args... AA) {
    switch (T->getKind()) {
#define TYPEREPR(CLASS, PARENT) \
    case TypeReprKind::CLASS: \
      return static_cast<ImplClass*>(this) \
        ->visit##CLASS##TypeRepr(static_cast<CLASS##TypeRepr*>(T), \
                                 ::std::forward<Args>(AA)...);
#include "language/AST/TypeReprNodes.def"
    }
    toolchain_unreachable("Not reachable, all cases handled");
  }

#define TYPEREPR(CLASS, PARENT) \
  TypeReprRetTy visit##CLASS##TypeRepr(CLASS##TypeRepr *T, Args... AA) {\
    return static_cast<ImplClass*>(this)->visit##PARENT(T, \
                                                 ::std::forward<Args>(AA)...); \
  }
#define ABSTRACT_TYPEREPR(CLASS, PARENT) TYPEREPR(CLASS, PARENT)
#include "language/AST/TypeReprNodes.def"

  AttributeRetTy visit(DeclAttribute *A, Args... AA) {
    switch (A->getKind()) {
#define DECL_ATTR(_, CLASS, ...)                                               \
  case DeclAttrKind::CLASS:                                                    \
    return static_cast<ImplClass *>(this)->visit##CLASS##Attr(                 \
        static_cast<CLASS##Attr *>(A), ::std::forward<Args>(AA)...);
#include "language/AST/DeclAttr.def"
    }
  }

#define DECL_ATTR(NAME,CLASS,...) \
  AttributeRetTy visit##CLASS##Attr(CLASS##Attr *A, Args... AA) { \
    return static_cast<ImplClass*>(this)->visitDeclAttribute(       \
             A, ::std::forward<Args>(AA)...);                       \
  }
#include "language/AST/DeclAttr.def"
  
  bool visit(ParameterList *PL) {
    return static_cast<ImplClass*>(this)->visitParameterList(PL);
  }
  
  bool visitParameterList(ParameterList *PL) { return false; }
};
  
  
template<typename ImplClass, typename ExprRetTy = void, typename... Args>
using ExprVisitor = ASTVisitor<ImplClass, ExprRetTy, void, void, void, void,
                               void, Args...>;

template<typename ImplClass, typename StmtRetTy = void, typename... Args>
using StmtVisitor = ASTVisitor<ImplClass, void, StmtRetTy, void, void, void,
                               void, Args...>;

template<typename ImplClass, typename DeclRetTy = void, typename... Args>
using DeclVisitor = ASTVisitor<ImplClass, void, void, DeclRetTy, void, void,
                               void, Args...>;

template<typename ImplClass, typename PatternRetTy = void, typename... Args>
using PatternVisitor = ASTVisitor<ImplClass, void,void,void, PatternRetTy, void,
                                  void, Args...>;

template<typename ImplClass, typename TypeReprRetTy = void, typename... Args>
using TypeReprVisitor = ASTVisitor<ImplClass, void,void,void,void,TypeReprRetTy,
                                   void, Args...>;

template<typename ImplClass, typename AttributeRetTy = void, typename... Args>
using AttributeVisitor = ASTVisitor<ImplClass, void,void,void,void,void,
                                    AttributeRetTy, Args...>;

} // end namespace language

#endif
