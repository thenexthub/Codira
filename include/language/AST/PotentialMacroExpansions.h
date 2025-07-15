//===--- PotentialMacroExpansions.h -----------------------------*- C++ -*-===//
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
//
//  This file defines a structure (\c PotentialMacroExpansions) to track
//  potential macro expansions within a given context.
//
//===----------------------------------------------------------------------===//
#ifndef LANGUAGE_AST_POTENTIAL_MACRO_EXPANSIONS_H
#define LANGUAGE_AST_POTENTIAL_MACRO_EXPANSIONS_H

#include "language/AST/Identifier.h"
#include "toolchain/ADT/PointerIntPair.h"
#include "toolchain/ADT/SmallPtrSet.h"

namespace language {

/// Describes the potential macro expansions within a given type or
/// extension context.
class PotentialMacroExpansions {
  enum {
    /// Whether there are any expanded macros.
    AnyExpandedMacros = 0x01,

    /// Whether any of the expanded macros introduces arbitrary names.
    IntroducesArbitraryNames = 0x02,
  };

  using NameSet = toolchain::SmallPtrSet<DeclName, 4>;

  /// Storage for the set of potential macro expansions.
  toolchain::PointerIntPair<NameSet *, 2, unsigned> Storage;

  /// Retrieve a pointer to the name set if there is one.
  const NameSet *getIntroducedNamesIfAvailable() const {
    return Storage.getPointer();
  }

  /// Get or create a nam
  NameSet &getOrCreateIntroducedNames() {
    if (auto nameSet = Storage.getPointer())
      return *nameSet;

    // Allocate a new set of introduced names.
    auto nameSet = new NameSet();
    Storage.setPointer(nameSet);
    return *nameSet;
  }

public:
  PotentialMacroExpansions() : Storage() { }

  PotentialMacroExpansions(const PotentialMacroExpansions &other)
    : Storage(nullptr, other.Storage.getInt())
  {
    if (auto otherNameSet = other.getIntroducedNamesIfAvailable()) {
      Storage.setPointer(new NameSet(*otherNameSet));
    }
  }

  PotentialMacroExpansions(PotentialMacroExpansions &&other)
    : Storage(other.Storage)
  {
    other.Storage.setPointer(nullptr);
  }

  PotentialMacroExpansions &operator=(const PotentialMacroExpansions &other) {
    PotentialMacroExpansions tmp(other);
    swap(tmp, *this);
    return *this;
  }

  PotentialMacroExpansions &operator=(PotentialMacroExpansions &&other) {
    if (&other != this) {
      Storage = other.Storage;
      other.Storage.setPointer(nullptr);
    }
    return *this;
  }

  ~PotentialMacroExpansions() {
    delete getIntroducedNamesIfAvailable();
  }

  /// Whether there are any expanded macros in this context.
  explicit operator bool() const { return hasAnyExpandedMacro(); }

  /// Whether there are any expanded macros in this context.
  bool hasAnyExpandedMacro() const {
    return Storage.getInt() & AnyExpandedMacros;
  }

  /// Note that we have expanded a macro.
  void noteExpandedMacro() {
    Storage.setInt(Storage.getInt() | AnyExpandedMacros);
  }

  /// Whether any expanded macro introduces arbitrary names.
  bool introducesArbitraryNames() const {
    return Storage.getInt() & IntroducesArbitraryNames;
  }

  /// Note that a macro expanded here introduced arbitrary names.
  void noteIntroducesArbitraryNames() {
    Storage.setInt(Storage.getInt() | IntroducesArbitraryNames);
  }

  /// Add a new introduced macro name.
  void addIntroducedMacroName(DeclName name) {
    getOrCreateIntroducedNames().insert(name.getBaseName());
  }

  /// Determine whether one should expand any macros in this context because
  /// they could introduce a declaration with the given name.
  bool shouldExpandForName(DeclName name) const {
    // If any macro produces arbitraty names, we need to expand it.
    if (introducesArbitraryNames())
      return true;

    auto introducedNames = getIntroducedNamesIfAvailable();
    if (!introducedNames)
      return false;

    return introducedNames->count(name.getBaseName());
  }

  friend bool operator==(const PotentialMacroExpansions &lhs,
                         const PotentialMacroExpansions &rhs) {
    // Check has-any-expanded-macro and introduces-arbitrary-names together.
    if (lhs.Storage.getInt() != rhs.Storage.getInt())
      return false;

    // If they introduced arbitrary names, ignore the name sets... they are
    // the same.
    if (lhs.introducesArbitraryNames())
      return true;

    // Both expanded macros and did not introduce arbitrary names, so we need
    // to check the actual names.
    auto lhsIntroducedNames = lhs.getIntroducedNamesIfAvailable();
    auto rhsIntroducedNames = rhs.getIntroducedNamesIfAvailable();

    auto lhsIntroducedNamesCount =
        lhsIntroducedNames ? lhsIntroducedNames->size() : 0;
    auto rhsIntroducedNamesCount =
        rhsIntroducedNames ? rhsIntroducedNames->size() : 0;
    if (lhsIntroducedNamesCount != rhsIntroducedNamesCount)
      return false;

    // Check whether both are empty.
    if (lhsIntroducedNamesCount == 0)
      return true;

    // Make sure all of the names of one are in the other.
    for (auto lhsName : *lhsIntroducedNames) {
      if (rhsIntroducedNames->count(lhsName) == 0)
        return false;
    }

    return true;
  }

  friend bool operator!=(const PotentialMacroExpansions &lhs,
                         const PotentialMacroExpansions &rhs) {
    return !(lhs == rhs);
  }

  friend void swap(
      PotentialMacroExpansions &lhs, PotentialMacroExpansions &rhs) {
    auto tmpStorage = lhs.Storage;
    lhs.Storage = rhs.Storage;
    rhs.Storage = tmpStorage;
  }
};

}

#endif // LANGUAGE_AST_POTENTIAL_MACRO_EXPANSIONS_H
