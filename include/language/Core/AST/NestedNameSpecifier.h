//===- NestedNameSpecifier.h - C++ nested name specifiers -------*- C++ -*-===//
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
//  This file completes the definition of the NestedNameSpecifier class.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_AST_NESTEDNAMESPECIFIER_H
#define LANGUAGE_CORE_AST_NESTEDNAMESPECIFIER_H

#include "language/Core/AST/Decl.h"
#include "language/Core/AST/NestedNameSpecifierBase.h"
#include "language/Core/AST/Type.h"
#include "language/Core/AST/TypeLoc.h"
#include "toolchain/ADT/DenseMapInfo.h"

namespace language::Core {

auto NestedNameSpecifier::getKind() const -> Kind {
  if (!isStoredKind()) {
    switch (getFlagKind()) {
    case FlagKind::Null:
      return Kind::Null;
    case FlagKind::Global:
      return Kind::Global;
    case FlagKind::Invalid:
      toolchain_unreachable("use of invalid NestedNameSpecifier");
    }
    toolchain_unreachable("unhandled FlagKind");
  }
  switch (auto [K, Ptr] = getStored(); K) {
  case StoredKind::Type:
    return Kind::Type;
  case StoredKind::NamespaceWithGlobal:
  case StoredKind::NamespaceWithNamespace:
    return Kind::Namespace;
  case StoredKind::NamespaceOrSuper:
    switch (static_cast<const Decl *>(Ptr)->getKind()) {
    case Decl::Namespace:
    case Decl::NamespaceAlias:
      return Kind::Namespace;
    case Decl::CXXRecord:
    case Decl::ClassTemplateSpecialization:
    case Decl::ClassTemplatePartialSpecialization:
      return Kind::MicrosoftSuper;
    default:
      toolchain_unreachable("unexpected decl kind");
    }
  }
  toolchain_unreachable("unknown StoredKind");
}

NestedNameSpecifier::NestedNameSpecifier(const Type *T)
    : NestedNameSpecifier({StoredKind::Type, T}) {
  assert(getKind() == Kind::Type);
}

auto NestedNameSpecifier::MakeNamespacePtrKind(
    const ASTContext &Ctx, const NamespaceBaseDecl *Namespace,
    NestedNameSpecifier Prefix) -> PtrKind {
  switch (Prefix.getKind()) {
  case Kind::Null:
    return {StoredKind::NamespaceOrSuper, Namespace};
  case Kind::Global:
    return {StoredKind::NamespaceWithGlobal, Namespace};
  case Kind::Namespace:
    return {StoredKind::NamespaceWithNamespace,
            MakeNamespaceAndPrefixStorage(Ctx, Namespace, Prefix)};
  case Kind::MicrosoftSuper:
  case Kind::Type:
    toolchain_unreachable("invalid prefix for namespace");
  }
  toolchain_unreachable("unhandled kind");
}

/// Builds a nested name specifier that names a namespace.
NestedNameSpecifier::NestedNameSpecifier(const ASTContext &Ctx,
                                         const NamespaceBaseDecl *Namespace,
                                         NestedNameSpecifier Prefix)
    : NestedNameSpecifier(MakeNamespacePtrKind(Ctx, Namespace, Prefix)) {
  assert(getKind() == Kind::Namespace);
}

/// Builds a nested name specifier that names a class through microsoft's
/// __super specifier.
NestedNameSpecifier::NestedNameSpecifier(CXXRecordDecl *RD)
    : NestedNameSpecifier({StoredKind::NamespaceOrSuper, RD}) {
  assert(getKind() == Kind::MicrosoftSuper);
}

CXXRecordDecl *NestedNameSpecifier::getAsRecordDecl() const {
  switch (getKind()) {
  case Kind::MicrosoftSuper:
    return getAsMicrosoftSuper();
  case Kind::Type:
    return getAsType()->getAsCXXRecordDecl();
  case Kind::Global:
  case Kind::Namespace:
  case Kind::Null:
    return nullptr;
  }
  toolchain_unreachable("Invalid NNS Kind!");
}

NestedNameSpecifier NestedNameSpecifier::getCanonical() const {
  switch (getKind()) {
  case NestedNameSpecifier::Kind::Null:
  case NestedNameSpecifier::Kind::Global:
  case NestedNameSpecifier::Kind::MicrosoftSuper:
    // These are canonical and unique.
    return *this;
  case NestedNameSpecifier::Kind::Namespace: {
    // A namespace is canonical; build a nested-name-specifier with
    // this namespace and no prefix.
    const NamespaceBaseDecl *ND = getAsNamespaceAndPrefix().Namespace;
    return NestedNameSpecifier(
        {StoredKind::NamespaceOrSuper, ND->getNamespace()->getCanonicalDecl()});
  }
  case NestedNameSpecifier::Kind::Type:
    return NestedNameSpecifier(
        getAsType()->getCanonicalTypeInternal().getTypePtr());
  }
  toolchain_unreachable("unhandled kind");
}

bool NestedNameSpecifier::isCanonical() const {
  return *this == getCanonical();
}

TypeLoc NestedNameSpecifierLoc::castAsTypeLoc() const {
  return TypeLoc(Qualifier.getAsType(), LoadPointer(/*Offset=*/0));
}

TypeLoc NestedNameSpecifierLoc::getAsTypeLoc() const {
  if (Qualifier.getKind() != NestedNameSpecifier::Kind::Type)
    return TypeLoc();
  return castAsTypeLoc();
}

unsigned
NestedNameSpecifierLoc::getLocalDataLength(NestedNameSpecifier Qualifier) {
  // Location of the trailing '::'.
  unsigned Length = sizeof(SourceLocation::UIntTy);

  switch (Qualifier.getKind()) {
  case NestedNameSpecifier::Kind::Global:
    // Nothing more to add.
    break;

  case NestedNameSpecifier::Kind::Namespace:
  case NestedNameSpecifier::Kind::MicrosoftSuper:
    // The location of the identifier or namespace name.
    Length += sizeof(SourceLocation::UIntTy);
    break;

  case NestedNameSpecifier::Kind::Type:
    // The "void*" that points at the TypeLoc data.
    // Note: the 'template' keyword is part of the TypeLoc.
    Length += sizeof(void *);
    break;

  case NestedNameSpecifier::Kind::Null:
    toolchain_unreachable("Expected a non-NULL qualifier");
  }

  return Length;
}

NamespaceAndPrefixLoc NestedNameSpecifierLoc::castAsNamespaceAndPrefix() const {
  auto [Namespace, Prefix] = Qualifier.getAsNamespaceAndPrefix();
  return {Namespace, NestedNameSpecifierLoc(Prefix, Data)};
}

NamespaceAndPrefixLoc NestedNameSpecifierLoc::getAsNamespaceAndPrefix() const {
  if (Qualifier.getKind() != NestedNameSpecifier::Kind::Namespace)
    return {};
  return castAsNamespaceAndPrefix();
}

unsigned NestedNameSpecifierLoc::getDataLength(NestedNameSpecifier Qualifier) {
  unsigned Length = 0;
  for (; Qualifier; Qualifier = Qualifier.getAsNamespaceAndPrefix().Prefix) {
    Length += getLocalDataLength(Qualifier);
    if (Qualifier.getKind() != NestedNameSpecifier::Kind::Namespace)
      break;
  }
  return Length;
}

unsigned NestedNameSpecifierLoc::getDataLength() const {
  return getDataLength(Qualifier);
}

SourceRange NestedNameSpecifierLoc::getLocalSourceRange() const {
  switch (auto Kind = Qualifier.getKind()) {
  case NestedNameSpecifier::Kind::Null:
    return SourceRange();
  case NestedNameSpecifier::Kind::Global:
    return LoadSourceLocation(/*Offset=*/0);
  case NestedNameSpecifier::Kind::Namespace:
  case NestedNameSpecifier::Kind::MicrosoftSuper: {
    unsigned Offset =
        Kind == NestedNameSpecifier::Kind::Namespace
            ? getDataLength(Qualifier.getAsNamespaceAndPrefix().Prefix)
            : 0;
    return SourceRange(
        LoadSourceLocation(Offset),
        LoadSourceLocation(Offset + sizeof(SourceLocation::UIntTy)));
  }
  case NestedNameSpecifier::Kind::Type: {
    // The "void*" that points at the TypeLoc data.
    // Note: the 'template' keyword is part of the TypeLoc.
    void *TypeData = LoadPointer(/*Offset=*/0);
    TypeLoc TL(Qualifier.getAsType(), TypeData);
    return SourceRange(TL.getBeginLoc(), LoadSourceLocation(sizeof(void *)));
  }
  }

  toolchain_unreachable("Invalid NNS Kind!");
}

SourceRange NestedNameSpecifierLoc::getSourceRange() const {
  return SourceRange(getBeginLoc(), getEndLoc());
}

SourceLocation NestedNameSpecifierLoc::getEndLoc() const {
  return getLocalSourceRange().getEnd();
}

/// Retrieve the location of the beginning of this
/// component of the nested-name-specifier.
SourceLocation NestedNameSpecifierLoc::getLocalBeginLoc() const {
  return getLocalSourceRange().getBegin();
}

/// Retrieve the location of the end of this component of the
/// nested-name-specifier.
SourceLocation NestedNameSpecifierLoc::getLocalEndLoc() const {
  return getLocalSourceRange().getEnd();
}

SourceRange NestedNameSpecifierLocBuilder::getSourceRange() const {
  return NestedNameSpecifierLoc(Representation, Buffer).getSourceRange();
}

} // namespace language::Core

namespace toolchain {

template <> struct DenseMapInfo<language::Core::NestedNameSpecifier> {
  static language::Core::NestedNameSpecifier getEmptyKey() { return std::nullopt; }

  static language::Core::NestedNameSpecifier getTombstoneKey() {
    return language::Core::NestedNameSpecifier::getInvalid();
  }

  static unsigned getHashValue(const language::Core::NestedNameSpecifier &V) {
    return hash_combine(V.getAsVoidPointer());
  }
};

template <> struct DenseMapInfo<language::Core::NestedNameSpecifierLoc> {
  using FirstInfo = DenseMapInfo<language::Core::NestedNameSpecifier>;
  using SecondInfo = DenseMapInfo<void *>;

  static language::Core::NestedNameSpecifierLoc getEmptyKey() {
    return language::Core::NestedNameSpecifierLoc(FirstInfo::getEmptyKey(),
                                         SecondInfo::getEmptyKey());
  }

  static language::Core::NestedNameSpecifierLoc getTombstoneKey() {
    return language::Core::NestedNameSpecifierLoc(FirstInfo::getTombstoneKey(),
                                         SecondInfo::getTombstoneKey());
  }

  static unsigned getHashValue(const language::Core::NestedNameSpecifierLoc &PairVal) {
    return hash_combine(
        FirstInfo::getHashValue(PairVal.getNestedNameSpecifier()),
        SecondInfo::getHashValue(PairVal.getOpaqueData()));
  }

  static bool isEqual(const language::Core::NestedNameSpecifierLoc &LHS,
                      const language::Core::NestedNameSpecifierLoc &RHS) {
    return LHS == RHS;
  }
};
} // namespace toolchain

#endif // LANGUAGE_CORE_AST_NESTEDNAMESPECIFIER_H
