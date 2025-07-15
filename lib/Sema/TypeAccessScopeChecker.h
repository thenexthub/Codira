//===--- TypeAccessScopeChecker.h - Computes access for Types ---*- C++ -*-===//
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

#ifndef LANGUAGE_SEMA_TYPEACCESSSCOPECHECKER_H
#define LANGUAGE_SEMA_TYPEACCESSSCOPECHECKER_H

#include "language/AST/Decl.h"
#include "language/AST/DeclContext.h"
#include "language/AST/SourceFile.h"
#include "language/AST/Type.h"
#include "language/AST/TypeDeclFinder.h"
#include "language/AST/TypeRepr.h"

namespace language {
class SourceFile;

/// Computes the access scope where a Type or TypeRepr is available, which is
/// the intersection of all the scopes where the declarations referenced in the
/// Type or TypeRepr are available.
class TypeAccessScopeChecker {
  const SourceFile *File;
  bool TreatUsableFromInlineAsPublic;

  std::optional<AccessScope> Scope = AccessScope::getPublic();

  TypeAccessScopeChecker(const DeclContext *useDC,
                         bool treatUsableFromInlineAsPublic)
      : File(useDC->getParentSourceFile()),
  TreatUsableFromInlineAsPublic(treatUsableFromInlineAsPublic) {}

  bool visitDecl(const ValueDecl *VD) {
    if (isa<GenericTypeParamDecl>(VD))
      return true;

    auto AS = VD->getFormalAccessScope(File, TreatUsableFromInlineAsPublic);
    Scope = Scope->intersectWith(AS);
    return Scope.has_value();
  }

public:
  static std::optional<AccessScope>
  getAccessScope(TypeRepr *TR, const DeclContext *useDC,
                 bool treatUsableFromInlineAsPublic = false) {
    TypeAccessScopeChecker checker(useDC, treatUsableFromInlineAsPublic);
    TR->walk(DeclRefTypeReprFinder([&](const DeclRefTypeRepr *typeRepr) {
      return checker.visitDecl(typeRepr->getBoundDecl());
    }));
    return checker.Scope;
  }

  static std::optional<AccessScope>
  getAccessScope(Type T, const DeclContext *useDC,
                 bool treatUsableFromInlineAsPublic = false) {
    TypeAccessScopeChecker checker(useDC, treatUsableFromInlineAsPublic);
    T.walk(SimpleTypeDeclFinder([&](const ValueDecl *VD) {
      if (checker.visitDecl(VD))
        return TypeWalker::Action::Continue;
      return TypeWalker::Action::Stop;
    }));

    return checker.Scope;
  }
};

} // end namespace language

#endif
