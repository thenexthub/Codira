//===--- TypeMemberVisitor.h - ASTVisitor specialization --------*- C++ -*-===//
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
// This file defines the curiously-recursive TypeMemberVisitor class
// and a few specializations thereof.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_AST_TYPEMEMBERVISITOR_H
#define LANGUAGE_AST_TYPEMEMBERVISITOR_H

#include "language/AST/ASTVisitor.h"

namespace language {
  
/// TypeMemberVisitor - This is a convenience adapter of DeclVisitor
/// which filters out a few common declaration kinds that are never
/// members of nominal types.
template<typename ImplClass, typename RetTy = void> 
class TypeMemberVisitor : public DeclVisitor<ImplClass, RetTy> {
protected:
  ImplClass &asImpl() { return static_cast<ImplClass&>(*this); }

public:
#define BAD_MEMBER(KIND) \
  RetTy visit##KIND##Decl(KIND##Decl *D) { \
    toolchain_unreachable(#KIND "Decls cannot be members of nominal types"); \
  }
  BAD_MEMBER(Extension)
  BAD_MEMBER(Import)
  BAD_MEMBER(TopLevelCode)
  BAD_MEMBER(Operator)
  BAD_MEMBER(PrecedenceGroup)
  BAD_MEMBER(Macro)
  BAD_MEMBER(Using)

  RetTy visitMacroExpansionDecl(MacroExpansionDecl *D) {
    // Expansion already visited as auxiliary decls.
    return RetTy();
  }

  /// A convenience method to visit all the members.
  void visitMembers(NominalTypeDecl *D) {
    for (Decl *member : D->getAllMembers()) {
      asImpl().visit(member);
    }
  }

  /// A convenience method to visit all the members in the implementation
  /// context.
  ///
  /// \seealso IterableDeclContext::getImplementationContext()
  void visitImplementationMembers(NominalTypeDecl *D) {
    for (Decl *member : D->getImplementationContext()->getAllMembers()) {
      asImpl().visit(member);
    }
    
    // If this is a main-interface @_objcImplementation extension and the class
    // has a synthesized destructor, visit it now.
    if (auto cd = dyn_cast_or_null<ClassDecl>(D)) {
      auto dd = cd->getDestructor();
      if (dd->getDeclContext() == cd && cd->getImplementationContext() != cd)
        asImpl().visit(dd);
    }
  }
};

template<typename ImplClass, typename RetTy = void>
class ClassMemberVisitor : public TypeMemberVisitor<ImplClass, RetTy> {
public:
  BAD_MEMBER(EnumElement)
  BAD_MEMBER(EnumCase)

  void visitMembers(ClassDecl *D) {
    TypeMemberVisitor<ImplClass, RetTy>::visitMembers(D);
  }

  void visitImplementationMembers(ClassDecl *D) {
    TypeMemberVisitor<ImplClass, RetTy>::visitImplementationMembers(D);
  }
};

#undef BAD_MEMBER

} // end namespace language
  
#endif
