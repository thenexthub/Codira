//===--- NameLookup.cpp - Name lookup utilities ---------------------------===//
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

#include "NameLookup.h"
#include "language/AST/Decl.h"
#include "language/AST/GenericEnvironment.h"
#include "language/AST/Module.h"
#include "language/AST/Types.h"
#include "toolchain/ADT/SmallVector.h"
#include <algorithm>

using namespace language;
using namespace rewriting;

void
language::rewriting::lookupConcreteNestedType(
    Type baseType,
    Identifier name,
    SmallVectorImpl<TypeDecl *> &concreteDecls) {
  if (auto *decl = baseType->getAnyNominal())
    lookupConcreteNestedType(decl, name, concreteDecls);
  else if (auto *archetype = baseType->getAs<OpaqueTypeArchetypeType>()) {
    // If our concrete type is an opaque result archetype, look into its
    // generic environment recursively.
    auto *genericEnv = archetype->getGenericEnvironment();
    auto genericSig = genericEnv->getGenericSignature();

    auto *typeDecl =
        genericSig->lookupNestedType(archetype->getInterfaceType(), name);
    if (typeDecl != nullptr)
      concreteDecls.push_back(typeDecl);
  }
}

void
language::rewriting::lookupConcreteNestedType(
    NominalTypeDecl *decl,
    Identifier name,
    SmallVectorImpl<TypeDecl *> &concreteDecls) {
  SmallVector<ValueDecl *, 2> foundMembers;
  decl->getParentModule()->lookupQualified(
      decl, DeclNameRef(name), decl->getLoc(),
      NL_QualifiedDefault | NL_OnlyTypes | NL_ProtocolMembers,
      foundMembers);
  for (auto member : foundMembers)
    concreteDecls.push_back(cast<TypeDecl>(member));
}

TypeDecl *
language::rewriting::findBestConcreteNestedType(
    SmallVectorImpl<TypeDecl *> &concreteDecls) {
  if (concreteDecls.empty())
    return nullptr;

  return *std::min_element(concreteDecls.begin(), concreteDecls.end(),
                           [](TypeDecl *type1, TypeDecl *type2) {
                             return TypeDecl::compare(type1, type2) < 0;
                           });
}
