//===--- Term.h - A term in the generics rewrite system ---------*- C++ -*-===//
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

#include "Symbol.h"
#include "toolchain/ADT/ArrayRef.h"
#include "toolchain/ADT/SmallVector.h"
#include <optional>

#ifndef LANGUAGE_RQM_TERM_H
#define LANGUAGE_RQM_TERM_H

namespace toolchain {
  class raw_ostream;
}

namespace language {

namespace rewriting {

/// A term is a sequence of one or more symbols.
///
/// The Term type is a uniqued, permanently-allocated representation,
/// used to represent terms in the rewrite rules themselves. See also
/// MutableTerm for the other representation.
///
/// The first symbol in the term must be a protocol, generic parameter, or
/// associated type symbol.
///
/// A layout, superclass or concrete type symbol must only appear at the
/// end of a term.
///
/// Out-of-line methods are documented in Term.cpp.
class Term final {
  friend class RewriteContext;

  struct Storage;

  const Storage *Ptr;

  Term(const Storage *ptr) : Ptr(ptr) {}

public:
  size_t size() const;

  const Symbol *begin() const;
  const Symbol *end() const;

  std::reverse_iterator<const Symbol *> rbegin() const;
  std::reverse_iterator<const Symbol *> rend() const;

  Symbol back() const;

  Symbol operator[](size_t index) const;

  /// Returns an opaque pointer that uniquely identifies this term.
  const void *getOpaquePointer() const {
    return Ptr;
  }

  static Term fromOpaquePointer(void *ptr) {
    return Term((Storage *) ptr);
  }

  static Term get(const MutableTerm &term, RewriteContext &ctx);

  const ProtocolDecl *getRootProtocol() const {
    return begin()->getRootProtocol();
  }

  bool containsNameSymbols() const;

  void dump(toolchain::raw_ostream &out) const;

  std::optional<int> compare(Term other, RewriteContext &ctx) const;

  friend bool operator==(Term lhs, Term rhs) {
    return lhs.Ptr == rhs.Ptr;
  }

  friend bool operator!=(Term lhs, Term rhs) {
    return !(lhs == rhs);
  }

  friend toolchain::raw_ostream &operator<<(toolchain::raw_ostream &out, Term term) {
    term.dump(out);
    return out;
  }
};

/// A term is a sequence of one or more symbols.
///
/// The MutableTerm type is a dynamically-allocated representation,
/// used to represent temporary values in simplification and completion.
/// See also Term for the other representation.
///
/// The first symbol in the term must be a protocol, generic parameter, or
/// associated type symbol.
///
/// A layout constraint symbol must only appear at the end of a term.
///
/// Out-of-line methods are documented in RewriteSystem.cpp.
class MutableTerm final {
  toolchain::SmallVector<Symbol, 3> Symbols;

public:
  /// Creates an empty term. At least one symbol must be added for the term
  /// to become valid.
  MutableTerm() {}

  explicit MutableTerm(const Symbol *begin,
                       const Symbol *end)
    : Symbols(begin, end) {}

  explicit MutableTerm(toolchain::SmallVector<Symbol, 3> &&symbols)
    : Symbols(std::move(symbols)) {}

  explicit MutableTerm(toolchain::ArrayRef<Symbol> symbols)
      : Symbols(symbols.begin(), symbols.end()) {}

  explicit MutableTerm(Term term)
    : Symbols(term.begin(), term.end()) {}

  void add(Symbol symbol) {
    Symbols.push_back(symbol);
  }

  void prepend(Symbol symbol) {
    Symbols.insert(Symbols.begin(), symbol);
  }

  void append(Term other) {
    Symbols.append(other.begin(), other.end());
  }

  void append(const MutableTerm &other) {
    Symbols.append(other.begin(), other.end());
  }

  void append(const Symbol *from, const Symbol *to) {
    Symbols.append(from, to);
  }

  std::optional<int> compare(const MutableTerm &other,
                             RewriteContext &ctx) const;

  bool empty() const { return Symbols.empty(); }

  size_t size() const { return Symbols.size(); }

  const ProtocolDecl *getRootProtocol() const {
    return begin()->getRootProtocol();
  }

  const Symbol *begin() const { return Symbols.begin(); }
  const Symbol *end() const { return Symbols.end(); }

  Symbol *begin() { return Symbols.begin(); }
  Symbol *end() { return Symbols.end(); }

  std::reverse_iterator<const Symbol *> rbegin() const { return Symbols.rbegin(); }
  std::reverse_iterator<const Symbol *> rend() const { return Symbols.rend(); }

  std::reverse_iterator<Symbol *> rbegin() { return Symbols.rbegin(); }
  std::reverse_iterator<Symbol *> rend() { return Symbols.rend(); }
  
  Symbol front() const {
    return Symbols.front();
  }

  Symbol back() const {
    return Symbols.back();
  }

  Symbol &back() {
    return Symbols.back();
  }

  Symbol operator[](size_t index) const {
    return Symbols[index];
  }

  Symbol &operator[](size_t index) {
    return Symbols[index];
  }

  void rewriteSubTerm(Symbol *from, Symbol *to, Term rhs);

  void dump(toolchain::raw_ostream &out) const;

  friend bool operator==(const MutableTerm &lhs, const MutableTerm &rhs) {
    if (lhs.size() != rhs.size())
      return false;

    return std::equal(lhs.begin(), lhs.end(), rhs.begin());
  }

  friend bool operator!=(const MutableTerm &lhs, const MutableTerm &rhs) {
    return !(lhs == rhs);
  }

  friend toolchain::raw_ostream &operator<<(toolchain::raw_ostream &out,
                                       const MutableTerm &term) {
    term.dump(out);
    return out;
  }
};

} // end namespace rewriting

} // end namespace language

namespace toolchain {
  template<> struct DenseMapInfo<language::rewriting::Term> {
    static language::rewriting::Term getEmptyKey() {
      return language::rewriting::Term::fromOpaquePointer(
        toolchain::DenseMapInfo<void *>::getEmptyKey());
    }
    static language::rewriting::Term getTombstoneKey() {
      return language::rewriting::Term::fromOpaquePointer(
        toolchain::DenseMapInfo<void *>::getTombstoneKey());
    }
    static unsigned getHashValue(language::rewriting::Term Val) {
      return DenseMapInfo<void *>::getHashValue(Val.getOpaquePointer());
    }
    static bool isEqual(language::rewriting::Term LHS,
                        language::rewriting::Term RHS) {
      return LHS == RHS;
    }
  };

  template<>
  struct PointerLikeTypeTraits<language::rewriting::Term> {
  public:
    static inline void *getAsVoidPointer(language::rewriting::Term Val) {
      return const_cast<void *>(Val.getOpaquePointer());
    }
    static inline language::rewriting::Term getFromVoidPointer(void *Ptr) {
      return language::rewriting::Term::fromOpaquePointer(Ptr);
    }
    enum { NumLowBitsAvailable = 1 };
  };
} // end namespace toolchain

#endif
