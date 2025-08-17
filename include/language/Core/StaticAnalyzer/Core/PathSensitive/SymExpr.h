//===- SymExpr.h - Management of Symbolic Values ----------------*- C++ -*-===//
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
//  This file defines SymExpr and SymbolData.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_STATICANALYZER_CORE_PATHSENSITIVE_SYMEXPR_H
#define LANGUAGE_CORE_STATICANALYZER_CORE_PATHSENSITIVE_SYMEXPR_H

#include "language/Core/AST/Type.h"
#include "language/Core/Basic/LLVM.h"
#include "toolchain/ADT/FoldingSet.h"
#include "toolchain/ADT/SmallVector.h"
#include "toolchain/ADT/iterator_range.h"
#include <cassert>

namespace language::Core {
namespace ento {

class MemRegion;

using SymbolID = unsigned;

/// Symbolic value. These values used to capture symbolic execution of
/// the program.
class SymExpr : public toolchain::FoldingSetNode {
  virtual void anchor();

public:
  enum Kind {
#define SYMBOL(Id, Parent) Id##Kind,
#define SYMBOL_RANGE(Id, First, Last) BEGIN_##Id = First, END_##Id = Last,
#include "language/Core/StaticAnalyzer/Core/PathSensitive/Symbols.def"
  };

private:
  Kind K;
  /// A unique identifier for this symbol.
  ///
  /// It is useful for SymbolData to easily differentiate multiple symbols, but
  /// also for "ephemeral" symbols, such as binary operations, because this id
  /// can be used for arranging constraints or equivalence classes instead of
  /// unstable pointer values.
  ///
  /// Note, however, that it can't be used in Profile because SymbolManager
  /// needs to compute Profile before allocating SymExpr.
  const SymbolID Sym;

protected:
  SymExpr(Kind k, SymbolID Sym) : K(k), Sym(Sym) {}

  static bool isValidTypeForSymbol(QualType T) {
    // FIXME: Depending on whether we choose to deprecate structural symbols,
    // this may become much stricter.
    return !T.isNull() && !T->isVoidType();
  }

  mutable unsigned Complexity = 0;

public:
  virtual ~SymExpr() = default;

  Kind getKind() const { return K; }

  /// Get a unique identifier for this symbol.
  /// The ID is unique across all SymExprs in a SymbolManager.
  /// They reflect the allocation order of these SymExprs,
  /// and are likely stable across runs.
  /// Used as a key in SymbolRef containers and as part of identity
  /// for SymbolData, e.g. SymbolConjured with ID = 7 is "conj_$7".
  SymbolID getSymbolID() const { return Sym; }

  virtual void dump() const;

  virtual void dumpToStream(raw_ostream &os) const {}

  virtual QualType getType() const = 0;
  virtual void Profile(toolchain::FoldingSetNodeID &profile) = 0;

  /// Iterator over symbols that the current symbol depends on.
  ///
  /// For SymbolData, it's the symbol itself; for expressions, it's the
  /// expression symbol and all the operands in it. Note, SymbolDerived is
  /// treated as SymbolData - the iterator will NOT visit the parent region.
  class symbol_iterator {
    SmallVector<const SymExpr *, 5> itr;

    void expand();

  public:
    symbol_iterator() = default;
    symbol_iterator(const SymExpr *SE);

    symbol_iterator &operator++();
    const SymExpr *operator*();

    bool operator==(const symbol_iterator &X) const;
    bool operator!=(const symbol_iterator &X) const;
  };

  toolchain::iterator_range<symbol_iterator> symbols() const {
    return toolchain::make_range(symbol_iterator(this), symbol_iterator());
  }

  virtual unsigned computeComplexity() const = 0;

  /// Find the region from which this symbol originates.
  ///
  /// Whenever the symbol was constructed to denote an unknown value of
  /// a certain memory region, return this region. This method
  /// allows checkers to make decisions depending on the origin of the symbol.
  /// Symbol classes for which the origin region is known include
  /// SymbolRegionValue which denotes the value of the region before
  /// the beginning of the analysis, and SymbolDerived which denotes the value
  /// of a certain memory region after its super region (a memory space or
  /// a larger record region) is default-bound with a certain symbol.
  /// It might return null.
  virtual const MemRegion *getOriginRegion() const { return nullptr; }
};

inline raw_ostream &operator<<(raw_ostream &os,
                               const language::Core::ento::SymExpr *SE) {
  SE->dumpToStream(os);
  return os;
}

using SymbolRef = const SymExpr *;
using SymbolRefSmallVectorTy = SmallVector<SymbolRef, 2>;

/// A symbol representing data which can be stored in a memory location
/// (region).
class SymbolData : public SymExpr {
  void anchor() override;

protected:
  SymbolData(Kind k, SymbolID sym) : SymExpr(k, sym) { assert(classof(this)); }

public:
  ~SymbolData() override = default;

  /// Get a string representation of the kind of the region.
  virtual StringRef getKindStr() const = 0;

  unsigned computeComplexity() const override {
    return 1;
  };

  // Implement isa<T> support.
  static bool classof(const SymExpr *SE) { return classof(SE->getKind()); }
  static constexpr bool classof(Kind K) {
    return K >= BEGIN_SYMBOLS && K <= END_SYMBOLS;
  }
};

} // namespace ento
} // namespace language::Core

#endif // LANGUAGE_CORE_STATICANALYZER_CORE_PATHSENSITIVE_SYMEXPR_H
