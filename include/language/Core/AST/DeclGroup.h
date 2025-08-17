//===- DeclGroup.h - Classes for representing groups of Decls ---*- C++ -*-===//
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
//  This file defines the DeclGroup, DeclGroupRef, and OwningDeclGroup classes.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_AST_DECLGROUP_H
#define LANGUAGE_CORE_AST_DECLGROUP_H

#include "toolchain/Support/TrailingObjects.h"
#include <cassert>
#include <cstdint>

namespace language::Core {

class ASTContext;
class Decl;

class DeclGroup final : private toolchain::TrailingObjects<DeclGroup, Decl *> {
  // FIXME: Include a TypeSpecifier object.
  unsigned NumDecls = 0;

private:
  DeclGroup() = default;
  DeclGroup(unsigned numdecls, Decl** decls);

public:
  friend TrailingObjects;

  static DeclGroup *Create(ASTContext &C, Decl **Decls, unsigned NumDecls);

  unsigned size() const { return NumDecls; }

  Decl *&operator[](unsigned i) { return getTrailingObjects(NumDecls)[i]; }

  Decl* const& operator[](unsigned i) const {
    return getTrailingObjects(NumDecls)[i];
  }
};

class DeclGroupRef {
  // Note this is not a PointerIntPair because we need the address of the
  // non-group case to be valid as a Decl** for iteration.
  enum Kind { SingleDeclKind=0x0, DeclGroupKind=0x1, Mask=0x1 };

  Decl* D = nullptr;

  Kind getKind() const {
    return (Kind) (reinterpret_cast<uintptr_t>(D) & Mask);
  }

public:
  DeclGroupRef() = default;
  explicit DeclGroupRef(Decl* d) : D(d) {}
  explicit DeclGroupRef(DeclGroup* dg)
    : D((Decl*) (reinterpret_cast<uintptr_t>(dg) | DeclGroupKind)) {}

  static DeclGroupRef Create(ASTContext &C, Decl **Decls, unsigned NumDecls) {
    if (NumDecls == 0)
      return DeclGroupRef();
    if (NumDecls == 1)
      return DeclGroupRef(Decls[0]);
    return DeclGroupRef(DeclGroup::Create(C, Decls, NumDecls));
  }

  using iterator = Decl **;
  using const_iterator = Decl * const *;

  bool isNull() const { return D == nullptr; }
  bool isSingleDecl() const { return getKind() == SingleDeclKind; }
  bool isDeclGroup() const { return getKind() == DeclGroupKind; }

  Decl *getSingleDecl() {
    assert(isSingleDecl() && "Isn't a single decl");
    return D;
  }
  const Decl *getSingleDecl() const {
    return const_cast<DeclGroupRef*>(this)->getSingleDecl();
  }

  DeclGroup &getDeclGroup() {
    assert(isDeclGroup() && "Isn't a declgroup");
    return *((DeclGroup*)(reinterpret_cast<uintptr_t>(D) & ~Mask));
  }
  const DeclGroup &getDeclGroup() const {
    return const_cast<DeclGroupRef*>(this)->getDeclGroup();
  }

  iterator begin() {
    if (isSingleDecl())
      return D ? &D : nullptr;
    return &getDeclGroup()[0];
  }

  iterator end() {
    if (isSingleDecl())
      return D ? &D+1 : nullptr;
    DeclGroup &G = getDeclGroup();
    return &G[0] + G.size();
  }

  const_iterator begin() const {
    if (isSingleDecl())
      return D ? &D : nullptr;
    return &getDeclGroup()[0];
  }

  const_iterator end() const {
    if (isSingleDecl())
      return D ? &D+1 : nullptr;
    const DeclGroup &G = getDeclGroup();
    return &G[0] + G.size();
  }

  void *getAsOpaquePtr() const { return D; }
  static DeclGroupRef getFromOpaquePtr(void *Ptr) {
    DeclGroupRef X;
    X.D = static_cast<Decl*>(Ptr);
    return X;
  }
};

} // namespace language::Core

namespace toolchain {

  // DeclGroupRef is "like a pointer", implement PointerLikeTypeTraits.
  template <typename T>
  struct PointerLikeTypeTraits;
  template <>
  struct PointerLikeTypeTraits<language::Core::DeclGroupRef> {
    static inline void *getAsVoidPointer(language::Core::DeclGroupRef P) {
      return P.getAsOpaquePtr();
    }

    static inline language::Core::DeclGroupRef getFromVoidPointer(void *P) {
      return language::Core::DeclGroupRef::getFromOpaquePtr(P);
    }

    static constexpr int NumLowBitsAvailable = 0;
  };

} // namespace toolchain

#endif // LANGUAGE_CORE_AST_DECLGROUP_H
