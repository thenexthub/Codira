//===--- Term.cpp - A term in the generics rewrite system -----------------===//
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

#include "language/AST/Decl.h"
#include "language/AST/Types.h"
#include "language/Basic/Assertions.h"
#include "toolchain/ADT/FoldingSet.h"
#include "toolchain/Support/raw_ostream.h"
#include <algorithm>
#include <vector>
#include "RewriteContext.h"
#include "Symbol.h"
#include "Term.h"

using namespace language;
using namespace rewriting;

/// Terms are uniqued and immutable, stored as a single pointer;
/// the Storage type is the allocated backing storage.
struct Term::Storage final
  : public toolchain::FoldingSetNode,
    public toolchain::TrailingObjects<Storage, Symbol> {
  friend class Symbol;

  unsigned Size;

  explicit Storage(unsigned size) : Size(size) {}

  size_t numTrailingObjects(OverloadToken<Symbol>) const {
    return Size;
  }

  MutableArrayRef<Symbol> getElements() {
    return {getTrailingObjects<Symbol>(), Size};
  }

  ArrayRef<Symbol> getElements() const {
    return {getTrailingObjects<Symbol>(), Size};
  }

  void Profile(toolchain::FoldingSetNodeID &id) const;
};

size_t Term::size() const { return Ptr->Size; }

const Symbol *Term::begin() const {
  return Ptr->getElements().begin();
}

const Symbol *Term::end() const {
  return Ptr->getElements().end();
}

std::reverse_iterator<const Symbol *> Term::rbegin() const {
  return Ptr->getElements().rbegin();
}

std::reverse_iterator<const Symbol *> Term::rend() const {
  return Ptr->getElements().rend();
}

Symbol Term::back() const {
  return Ptr->getElements().back();
}

Symbol Term::operator[](size_t index) const {
  return Ptr->getElements()[index];
}

void Term::dump(toolchain::raw_ostream &out) const {
  MutableTerm(*this).dump(out);
}

Term Term::get(const MutableTerm &mutableTerm, RewriteContext &ctx) {
  unsigned size = mutableTerm.size();
  DEBUG_ASSERT(size > 0 && "Term must have at least one symbol");

  toolchain::FoldingSetNodeID id;
  id.AddInteger(size);
  for (auto symbol : mutableTerm)
    id.AddPointer(symbol.getOpaquePointer());

  void *insertPos = nullptr;
  if (auto *term = ctx.Terms.FindNodeOrInsertPos(id, insertPos))
    return term;

  void *mem = ctx.Allocator.Allocate(
      Storage::totalSizeToAlloc<Symbol>(size),
      alignof(Storage));
  auto *term = new (mem) Storage(size);
  for (unsigned i = 0; i < size; ++i)
    term->getElements()[i] = mutableTerm[i];

  ctx.Terms.InsertNode(term, insertPos);
  ctx.TermHistogram.add(size);

  return term;
}

void Term::Storage::Profile(toolchain::FoldingSetNodeID &id) const {
  id.AddInteger(Size);

  for (auto symbol : getElements())
    id.AddPointer(symbol.getOpaquePointer());
}

bool Term::containsNameSymbols() const {
  for (auto symbol : *this) {
    if (symbol.getKind() == Symbol::Kind::Name)
      return true;
  }

  return false;
}

/// Weighted shortlex order on symbol ranges, used for implementing
/// Term::compare() and MutableTerm::compare().
///
/// We first compute a weight vector for both terms and compare the
/// vectors lexicographically:
/// - Weight of generic param symbols
/// - Number of name symbols
/// - Number of element symbols
///
/// If the terms have the same weight, we compare length.
///
/// If the terms have the same weight and length, we perform a
/// lexicographic comparison on symbols.
///
static std::optional<int> compareImpl(const Symbol *lhsBegin,
                                      const Symbol *lhsEnd,
                                      const Symbol *rhsBegin,
                                      const Symbol *rhsEnd,
                                      RewriteContext &ctx) {
  ASSERT(lhsBegin != lhsEnd);
  ASSERT(rhsBegin != rhsEnd);

  // First compare weights on generic parameters. The implicit
  // assumption here is we don't form terms with generic parameter
  // symbols in the middle, which is true. Otherwise, we'd need
  // to add up their weights like we do below for name symbols,
  // of course.
  if (lhsBegin->getKind() == Symbol::Kind::GenericParam &&
      rhsBegin->getKind() == Symbol::Kind::GenericParam) {
    unsigned lhsWeight = lhsBegin->getGenericParam()->getWeight();
    unsigned rhsWeight = rhsBegin->getGenericParam()->getWeight();
    if (lhsWeight != rhsWeight)
      return lhsWeight > rhsWeight ? 1 : -1;
  }

  // Compare the number of name and pack element symbols.
  unsigned lhsNameCount = 0;
  unsigned lhsPackElementCount = 0;
  for (auto *iter = lhsBegin; iter != lhsEnd; ++iter) {
    if (iter->getKind() == Symbol::Kind::Name)
      ++lhsNameCount;

    if (iter->getKind() == Symbol::Kind::PackElement)
      ++lhsPackElementCount;
  }

  unsigned rhsNameCount = 0;
  unsigned rhsPackElementCount = 0;
  for (auto *iter = rhsBegin; iter != rhsEnd; ++iter) {
    if (iter->getKind() == Symbol::Kind::Name)
      ++rhsNameCount;

    if (iter->getKind() == Symbol::Kind::PackElement)
      ++rhsPackElementCount;
  }

  // A term with more pack element symbols orders after a term with
  // fewer pack element symbols.
  if (lhsPackElementCount != rhsPackElementCount)
    return lhsPackElementCount > rhsPackElementCount ? 1 : -1;

  // A term with more name symbols orders after a term with fewer name symbols.
  if (lhsNameCount != rhsNameCount)
    return lhsNameCount > rhsNameCount ? 1 : -1;

  // Next, compare term length.
  unsigned lhsSize = (lhsEnd - lhsBegin);
  unsigned rhsSize = (rhsEnd - rhsBegin);

  // A longer term orders after a shorter term.
  if (lhsSize != rhsSize)
    return lhsSize < rhsSize ? -1 : 1;

  // Finally, compare symbols pairwise.
  while (lhsBegin != lhsEnd) {
    auto lhs = *lhsBegin;
    auto rhs = *rhsBegin;

    ++lhsBegin;
    ++rhsBegin;

    std::optional<int> result = lhs.compare(rhs, ctx);
    if (!result.has_value() || *result != 0) {
      DEBUG_ASSERT(lhs != rhs);
      return result;
    }

    DEBUG_ASSERT(lhs == rhs);
  }

  return 0;
}

/// Reduction order on terms. Returns None if the terms are identical except
/// for an incomparable superclass or concrete type symbol at the end.
std::optional<int> Term::compare(Term other, RewriteContext &ctx) const {
  return compareImpl(begin(), end(), other.begin(), other.end(), ctx);
}

/// Reduction order on mutable terms. Returns None if the terms are identical
/// except for an incomparable superclass or concrete type symbol at the end.
std::optional<int> MutableTerm::compare(const MutableTerm &other,
                                        RewriteContext &ctx) const {
  return compareImpl(begin(), end(), other.begin(), other.end(), ctx);
}

/// Replace the subterm in the range [from,to) of this term with \p rhs.
void MutableTerm::rewriteSubTerm(Symbol *from, Symbol *to, Term rhs) {
  auto oldSize = size();
  size_t lhsLength = (size_t)(to - from);

  if (lhsLength == rhs.size()) {
    // Copy the RHS to the LHS.
    auto newTo = std::copy(rhs.begin(), rhs.end(), from);

    // The RHS has the same length as the LHS, so we're done.
    DEBUG_ASSERT(newTo == to);
    (void) newTo;
  } else if (lhsLength > rhs.size()) {
    // Copy the RHS to the LHS.
    auto newTo = std::copy(rhs.begin(), rhs.end(), from);

    // Shorten the term.
    Symbols.erase(newTo, to);
  } else {
    DEBUG_ASSERT(lhsLength < rhs.size());

    // Copy the LHS-sized prefix of RHS to the LHS.
    auto newTo = std::copy_n(rhs.begin(), lhsLength, from);
    DEBUG_ASSERT(newTo == to);

    // Insert the remainder of the RHS term.
    Symbols.insert(to, rhs.begin() + lhsLength, rhs.end());
  }

  DEBUG_ASSERT(size() == oldSize - lhsLength + rhs.size());
}

void MutableTerm::dump(toolchain::raw_ostream &out) const {
  bool first = true;

  for (auto symbol : Symbols) {
    if (!first)
      out << ".";
    else
      first = false;

    symbol.dump(out);
  }
}
