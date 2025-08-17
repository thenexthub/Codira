//===- ASTUnresolvedSet.h - Unresolved sets of declarations -----*- C++ -*-===//
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
//  This file provides an UnresolvedSet-like class, whose contents are
//  allocated using the allocator associated with an ASTContext.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_AST_ASTUNRESOLVEDSET_H
#define LANGUAGE_CORE_AST_ASTUNRESOLVEDSET_H

#include "language/Core/AST/ASTVector.h"
#include "language/Core/AST/DeclAccessPair.h"
#include "language/Core/AST/DeclID.h"
#include "language/Core/AST/UnresolvedSet.h"
#include "language/Core/Basic/Specifiers.h"
#include <cassert>
#include <cstdint>

namespace language::Core {

class NamedDecl;

/// An UnresolvedSet-like class which uses the ASTContext's allocator.
class ASTUnresolvedSet {
  friend class LazyASTUnresolvedSet;

  struct DeclsTy : ASTVector<DeclAccessPair> {
    DeclsTy() = default;
    DeclsTy(ASTContext &C, unsigned N) : ASTVector<DeclAccessPair>(C, N) {}

    bool isLazy() const { return getTag(); }
    void setLazy(bool Lazy) { setTag(Lazy); }
  };

  DeclsTy Decls;

public:
  ASTUnresolvedSet() = default;
  ASTUnresolvedSet(ASTContext &C, unsigned N) : Decls(C, N) {}

  using iterator = UnresolvedSetIterator;
  using const_iterator = UnresolvedSetIterator;

  iterator begin() { return iterator(Decls.begin()); }
  iterator end() { return iterator(Decls.end()); }

  const_iterator begin() const { return const_iterator(Decls.begin()); }
  const_iterator end() const { return const_iterator(Decls.end()); }

  void addDecl(ASTContext &C, NamedDecl *D, AccessSpecifier AS) {
    Decls.push_back(DeclAccessPair::make(D, AS), C);
  }

  void addLazyDecl(ASTContext &C, GlobalDeclID ID, AccessSpecifier AS) {
    Decls.push_back(DeclAccessPair::makeLazy(ID.getRawValue(), AS), C);
  }

  /// Replaces the given declaration with the new one, once.
  ///
  /// \return true if the set changed
  bool replace(const NamedDecl *Old, NamedDecl *New, AccessSpecifier AS) {
    for (DeclsTy::iterator I = Decls.begin(), E = Decls.end(); I != E; ++I) {
      if (I->getDecl() == Old) {
        I->set(New, AS);
        return true;
      }
    }
    return false;
  }

  void erase(unsigned I) {
    if (I == Decls.size() - 1)
      Decls.pop_back();
    else
      Decls[I] = Decls.pop_back_val();
  }

  void clear() { Decls.clear(); }

  bool empty() const { return Decls.empty(); }
  unsigned size() const { return Decls.size(); }

  void reserve(ASTContext &C, unsigned N) {
    Decls.reserve(C, N);
  }

  void append(ASTContext &C, iterator I, iterator E) {
    Decls.append(C, I.I, E.I);
  }

  DeclAccessPair &operator[](unsigned I) { return Decls[I]; }
  const DeclAccessPair &operator[](unsigned I) const { return Decls[I]; }
};

/// An UnresolvedSet-like class that might not have been loaded from the
/// external AST source yet.
class LazyASTUnresolvedSet {
  mutable ASTUnresolvedSet Impl;

  void getFromExternalSource(ASTContext &C) const;

public:
  ASTUnresolvedSet &get(ASTContext &C) const {
    if (Impl.Decls.isLazy())
      getFromExternalSource(C);
    return Impl;
  }

  void reserve(ASTContext &C, unsigned N) { Impl.reserve(C, N); }

  void addLazyDecl(ASTContext &C, GlobalDeclID ID, AccessSpecifier AS) {
    assert(Impl.empty() || Impl.Decls.isLazy());
    Impl.Decls.setLazy(true);
    Impl.addLazyDecl(C, ID, AS);
  }
};

} // namespace language::Core

#endif // LANGUAGE_CORE_AST_ASTUNRESOLVEDSET_H
