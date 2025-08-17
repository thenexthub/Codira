//===- UnresolvedSet.h - Unresolved sets of declarations --------*- C++ -*-===//
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
//  This file defines the UnresolvedSet class, which is used to store
//  collections of declarations in the AST.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_AST_UNRESOLVEDSET_H
#define LANGUAGE_CORE_AST_UNRESOLVEDSET_H

#include "language/Core/AST/DeclAccessPair.h"
#include "language/Core/Basic/LLVM.h"
#include "language/Core/Basic/Specifiers.h"
#include "toolchain/ADT/ArrayRef.h"
#include "toolchain/ADT/SmallVector.h"
#include "toolchain/ADT/iterator.h"
#include <cstddef>
#include <iterator>

namespace language::Core {

class NamedDecl;

/// The iterator over UnresolvedSets.  Serves as both the const and
/// non-const iterator.
class UnresolvedSetIterator : public toolchain::iterator_adaptor_base<
                                  UnresolvedSetIterator, DeclAccessPair *,
                                  std::random_access_iterator_tag, NamedDecl *,
                                  std::ptrdiff_t, NamedDecl *, NamedDecl *> {
  friend class ASTUnresolvedSet;
  friend class OverloadExpr;
  friend class UnresolvedSetImpl;

  explicit UnresolvedSetIterator(DeclAccessPair *Iter)
      : iterator_adaptor_base(Iter) {}
  explicit UnresolvedSetIterator(const DeclAccessPair *Iter)
      : iterator_adaptor_base(const_cast<DeclAccessPair *>(Iter)) {}

public:
  // Work around a bug in MSVC 2013 where explicitly default constructed
  // temporaries with defaulted ctors are not zero initialized.
  UnresolvedSetIterator() : iterator_adaptor_base(nullptr) {}

  uint64_t getDeclID() const { return I->getDeclID(); }
  NamedDecl *getDecl() const { return I->getDecl(); }
  void setDecl(NamedDecl *ND) const { return I->setDecl(ND); }
  AccessSpecifier getAccess() const { return I->getAccess(); }
  void setAccess(AccessSpecifier AS) { I->setAccess(AS); }
  const DeclAccessPair &getPair() const { return *I; }

  NamedDecl *operator*() const { return getDecl(); }
  NamedDecl *operator->() const { return **this; }
};

/// A set of unresolved declarations.
class UnresolvedSetImpl {
  using DeclsTy = SmallVectorImpl<DeclAccessPair>;

  // Don't allow direct construction, and only permit subclassing by
  // UnresolvedSet.
private:
  template <unsigned N> friend class UnresolvedSet;

  UnresolvedSetImpl() = default;
  UnresolvedSetImpl(const UnresolvedSetImpl &) = default;
  UnresolvedSetImpl &operator=(const UnresolvedSetImpl &) = default;

  UnresolvedSetImpl(UnresolvedSetImpl &&) = default;
  UnresolvedSetImpl &operator=(UnresolvedSetImpl &&) = default;

public:
  // We don't currently support assignment through this iterator, so we might
  // as well use the same implementation twice.
  using iterator = UnresolvedSetIterator;
  using const_iterator = UnresolvedSetIterator;

  iterator begin() { return iterator(decls().begin()); }
  iterator end() { return iterator(decls().end()); }

  const_iterator begin() const { return const_iterator(decls().begin()); }
  const_iterator end() const { return const_iterator(decls().end()); }

  ArrayRef<DeclAccessPair> pairs() const { return decls(); }

  void addDecl(NamedDecl *D) {
    addDecl(D, AS_none);
  }

  void addDecl(NamedDecl *D, AccessSpecifier AS) {
    decls().push_back(DeclAccessPair::make(D, AS));
  }

  /// Replaces the given declaration with the new one, once.
  ///
  /// \return true if the set changed
  bool replace(const NamedDecl* Old, NamedDecl *New) {
    for (DeclsTy::iterator I = decls().begin(), E = decls().end(); I != E; ++I)
      if (I->getDecl() == Old)
        return (I->setDecl(New), true);
    return false;
  }

  /// Replaces the declaration at the given iterator with the new one,
  /// preserving the original access bits.
  void replace(iterator I, NamedDecl *New) { I.I->setDecl(New); }

  void replace(iterator I, NamedDecl *New, AccessSpecifier AS) {
    I.I->set(New, AS);
  }

  void erase(unsigned I) {
    auto val = decls().pop_back_val();
    if (I < size())
      decls()[I] = val;
  }

  void erase(iterator I) {
    auto val = decls().pop_back_val();
    if (I != end())
      *I.I = val;
  }

  void setAccess(iterator I, AccessSpecifier AS) { I.I->setAccess(AS); }

  void clear() { decls().clear(); }
  void truncate(unsigned N) { decls().truncate(N); }

  bool empty() const { return decls().empty(); }
  unsigned size() const { return decls().size(); }

  void append(iterator I, iterator E) { decls().append(I.I, E.I); }

  template<typename Iter> void assign(Iter I, Iter E) { decls().assign(I, E); }

  DeclAccessPair &operator[](unsigned I) { return decls()[I]; }
  const DeclAccessPair &operator[](unsigned I) const { return decls()[I]; }

private:
  // These work because the only permitted subclass is UnresolvedSetImpl

  DeclsTy &decls() {
    return *reinterpret_cast<DeclsTy*>(this);
  }
  const DeclsTy &decls() const {
    return *reinterpret_cast<const DeclsTy*>(this);
  }
};

/// A set of unresolved declarations.
template <unsigned InlineCapacity> class UnresolvedSet :
    public UnresolvedSetImpl {
  SmallVector<DeclAccessPair, InlineCapacity> Decls;
};


} // namespace language::Core

#endif // LANGUAGE_CORE_AST_UNRESOLVEDSET_H
