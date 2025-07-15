//===--- TypeExpansionContext.h - Codira Type Expansion Context --*- C++ -*-===//
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
// This file defines the TypeExpansionContext class.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_TYPEEXPANSIONCONTEXT_H
#define LANGUAGE_TYPEEXPANSIONCONTEXT_H

#include "language/AST/ResilienceExpansion.h"
#include "toolchain/ADT/DenseMap.h"

namespace language {
  class DeclContext;
  class SILFunction;

/// Describes the context in which SIL types should eventually be expanded.
/// Required for lowering resilient types and deciding whether to look through
/// opaque result types to their underlying type.
class TypeExpansionContext {
  ResilienceExpansion expansion;
  // The context (module, function, ...) we are expanding the type in.
  const DeclContext *inContext;
  // Is the context in which we are expanding in the whole module.
  bool isContextWholeModule;

  // The minimal expansion.
  TypeExpansionContext() {
    inContext = nullptr;
    expansion = ResilienceExpansion::Minimal;
    isContextWholeModule = false;
  }

public:

  // Infer the expansion for the SIL function.
  TypeExpansionContext(const SILFunction &f);

  TypeExpansionContext(ResilienceExpansion expansion,
                       const DeclContext *inContext, bool isWholeModuleContext)
      : expansion(expansion), inContext(inContext),
        isContextWholeModule(isWholeModuleContext) {}

  ResilienceExpansion getResilienceExpansion() const { return expansion; }

  const DeclContext *getContext() const { return inContext; }

  bool isWholeModuleContext() const { return isContextWholeModule; }

  bool shouldLookThroughOpaqueTypeArchetypes() const {
    return inContext != nullptr;
  }

  bool isMinimal() const { return *this == TypeExpansionContext(); }

  static TypeExpansionContext minimal() {
    return TypeExpansionContext();
  }

  static TypeExpansionContext maximal(const DeclContext *inContext,
                                      bool isContextWholeModule) {
    return TypeExpansionContext(ResilienceExpansion::Maximal, inContext,
                                isContextWholeModule);
  }

  static TypeExpansionContext maximalResilienceExpansionOnly() {
    return maximal(nullptr, false);
  }

  static TypeExpansionContext
  noOpaqueTypeArchetypesSubstitution(ResilienceExpansion expansion) {
    return TypeExpansionContext(expansion, nullptr, false);
  }

  bool operator==(const TypeExpansionContext &other) const {
    assert(other.inContext != this->inContext ||
           other.isContextWholeModule == this->isContextWholeModule);
    return other.inContext == this->inContext &&
      other.expansion == this->expansion;
  }

  bool operator!=(const TypeExpansionContext &other) const {
    return !operator==(other);
  }

  bool operator<(const TypeExpansionContext other) const {
    assert(other.inContext != this->inContext ||
           other.isContextWholeModule == this->isContextWholeModule);
    if (this->expansion == other.expansion)
      return this->inContext < other.inContext;
    return this->expansion < other.expansion;
  }

  bool operator>(const TypeExpansionContext other) const {
    assert(other.inContext != this->inContext ||
           other.isContextWholeModule == this->isContextWholeModule);
    if (this->expansion == other.expansion)
      return this->inContext > other.inContext;
    return this->expansion > other.expansion;
  }

  uintptr_t getHashKey() const {
    uintptr_t key = (uintptr_t)inContext | (uintptr_t)expansion;
    return key;
  }
};

} // namespace language

namespace toolchain {
template <> struct DenseMapInfo<language::TypeExpansionContext> {
  using TypeExpansionContext = language::TypeExpansionContext;

  static TypeExpansionContext getEmptyKey() {
    return TypeExpansionContext(
        language::ResilienceExpansion::Minimal,
        reinterpret_cast<language::DeclContext *>(
            DenseMapInfo<language::DeclContext *>::getEmptyKey()),
        false);
  }
  static TypeExpansionContext getTombstoneKey() {
    return TypeExpansionContext(
        language::ResilienceExpansion::Minimal,
        reinterpret_cast<language::DeclContext *>(
            DenseMapInfo<language::DeclContext *>::getTombstoneKey()),
        false);
  }

  static unsigned getHashValue(TypeExpansionContext val) {
    return DenseMapInfo<uintptr_t>::getHashValue(val.getHashKey());
  }

  static bool isEqual(TypeExpansionContext LHS, TypeExpansionContext RHS) {
    return LHS == RHS;
  }
};
} // namespace toolchain

#endif
