//===------------------------ MapLattice.h ----------------------*- C++ -*-===//
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
//  This file defines a parameterized lattice that maps keys to individual
//  lattice elements (of the parameter lattice type). A typical usage is lifting
//  a particular lattice to all variables in a lexical scope.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_ANALYSIS_FLOWSENSITIVE__MAPLATTICE_H
#define LANGUAGE_CORE_ANALYSIS_FLOWSENSITIVE__MAPLATTICE_H

#include <ostream>
#include <string>
#include <utility>

#include "DataflowAnalysis.h"
#include "language/Core/AST/Decl.h"
#include "language/Core/Analysis/FlowSensitive/DataflowLattice.h"
#include "toolchain/ADT/DenseMap.h"
#include "toolchain/ADT/StringRef.h"

namespace language::Core {
namespace dataflow {

/// A lattice that maps keys to individual lattice elements. When instantiated
/// with an `ElementLattice` that is a bounded semi-lattice, `MapLattice` is
/// itself a bounded semi-lattice, so long as the user limits themselves to a
/// finite number of keys. In that case, `top` is (implicitly), the map
/// containing all valid keys mapped to `top` of `ElementLattice`.
///
/// Requirements on `ElementLattice`:
/// * Provides standard declarations of a bounded semi-lattice.
template <typename Key, typename ElementLattice> class MapLattice {
  using Container = toolchain::DenseMap<Key, ElementLattice>;
  Container C;

public:
  using key_type = Key;
  using mapped_type = ElementLattice;
  using value_type = typename Container::value_type;
  using iterator = typename Container::iterator;
  using const_iterator = typename Container::const_iterator;

  MapLattice() = default;

  explicit MapLattice(Container C) : C{std::move(C)} {};

  // The `bottom` element is the empty map.
  static MapLattice bottom() { return MapLattice(); }

  std::pair<iterator, bool>
  insert(const std::pair<const key_type, mapped_type> &P) {
    return C.insert(P);
  }

  std::pair<iterator, bool> insert(std::pair<const key_type, mapped_type> &&P) {
    return C.insert(std::move(P));
  }

  unsigned size() const { return C.size(); }
  bool empty() const { return C.empty(); }

  iterator begin() { return C.begin(); }
  iterator end() { return C.end(); }
  const_iterator begin() const { return C.begin(); }
  const_iterator end() const { return C.end(); }

  // Equality is direct equality of underlying map entries. One implication of
  // this definition is that a map with (only) keys that map to bottom is not
  // equal to the empty map.
  friend bool operator==(const MapLattice &LHS, const MapLattice &RHS) {
    return LHS.C == RHS.C;
  }

  friend bool operator!=(const MapLattice &LHS, const MapLattice &RHS) {
    return !(LHS == RHS);
  }

  bool contains(const key_type &K) const { return C.find(K) != C.end(); }

  iterator find(const key_type &K) { return C.find(K); }
  const_iterator find(const key_type &K) const { return C.find(K); }

  mapped_type &operator[](const key_type &K) { return C[K]; }

  /// If an entry exists in one map but not the other, the missing entry is
  /// treated as implicitly mapping to `bottom`. So, the joined map contains the
  /// entry as it was in the source map.
  LatticeJoinEffect join(const MapLattice &Other) {
    LatticeJoinEffect Effect = LatticeJoinEffect::Unchanged;
    for (const auto &O : Other.C) {
      auto It = C.find(O.first);
      if (It == C.end()) {
        C.insert(O);
        Effect = LatticeJoinEffect::Changed;
      } else if (It->second.join(O.second) == LatticeJoinEffect::Changed)
        Effect = LatticeJoinEffect::Changed;
    }
    return Effect;
  }
};

/// Convenience alias that captures the common use of map lattices to model
/// in-scope variables.
template <typename ElementLattice>
using VarMapLattice = MapLattice<const language::Core::VarDecl *, ElementLattice>;

template <typename Key, typename ElementLattice>
std::ostream &
operator<<(std::ostream &Os,
           const language::Core::dataflow::MapLattice<Key, ElementLattice> &M) {
  std::string Separator;
  Os << "{";
  for (const auto &E : M) {
    Os << std::exchange(Separator, ", ") << E.first << " => " << E.second;
  }
  Os << "}";
  return Os;
}

template <typename ElementLattice>
std::ostream &
operator<<(std::ostream &Os,
           const language::Core::dataflow::VarMapLattice<ElementLattice> &M) {
  std::string Separator;
  Os << "{";
  for (const auto &E : M) {
    Os << std::exchange(Separator, ", ") << E.first->getName().str() << " => "
       << E.second;
  }
  Os << "}";
  return Os;
}
} // namespace dataflow
} // namespace language::Core

#endif // LANGUAGE_CORE_ANALYSIS_FLOWSENSITIVE__MAPLATTICE_H
