//===--- SubstitutionMap.h - Codira Substitution Map ASTs --------*- C++ -*-===//
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
// This file defines the SubstitutionMap class.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_AST_SUBSTITUTION_MAP_H
#define LANGUAGE_AST_SUBSTITUTION_MAP_H

#include "language/AST/GenericSignature.h"
#include "language/AST/ProtocolConformanceRef.h"
#include "language/AST/Type.h"
#include "language/AST/TypeExpansionContext.h"
#include "language/Basic/Debug.h"
#include "toolchain/ADT/ArrayRef.h"
#include "toolchain/ADT/DenseMapInfo.h"
#include <optional>

namespace toolchain {
  class FoldingSetNodeID;
}

namespace language {

class GenericEnvironment;
class GenericParamList;
class RecursiveTypeProperties;
class SubstitutableType;
typedef CanTypeWrapper<GenericTypeParamType> CanGenericTypeParamType;

template<class Type> class CanTypeWrapper;
typedef CanTypeWrapper<SubstitutableType> CanSubstitutableType;

/// SubstitutionMap is a data structure type that describes the mapping of
/// abstract types to replacement types, together with associated conformances
/// to use for deriving nested types and conformances.
///
/// Substitution maps are primarily used when performing substitutions into
/// any entity that can reference type parameters, e.g., types (via
/// Type::subst()) and conformances (via ProtocolConformanceRef::subst()).
///
/// SubstitutionMaps are constructed by calling the an overload of the static
/// method \c SubstitutionMap::get(). However, most substitution maps are
/// computed using higher-level entry points such as
/// TypeBase::getContextSubstitutionMap().
///
/// Substitution maps are ASTContext-allocated and are uniqued on construction,
/// so they can be used as fields in AST nodes.
class SubstitutionMap {
public:
  /// Stored data for a substitution map, which uses tail allocation for the
  /// replacement types and conformances.
  class Storage;

private:
  /// The storage needed to describe the set of substitutions.
  ///
  /// When null, this substitution map is empty, having neither a generic
  /// signature nor any replacement types/conformances.
  Storage *storage = nullptr;

  /// Form a substitution map for the given generic signature with the
  /// specified replacement types and conformances.
  SubstitutionMap(GenericSignature genericSig,
                  ArrayRef<Type> replacementTypes,
                  ArrayRef<ProtocolConformanceRef> conformances);

  explicit SubstitutionMap(Storage *storage) : storage(storage) { }

public:
  /// Build an empty substitution map.
  SubstitutionMap() { }

  /// The primitive constructor.
  static SubstitutionMap get(GenericSignature genericSig,
                             ArrayRef<Type> replacementTypes,
                             ArrayRef<ProtocolConformanceRef> conformances) {
    return SubstitutionMap(genericSig, replacementTypes, conformances);
  }

  /// Translate a substitution map from one generic signature to another
  /// "compatible" one. Think carefully before using this.
  static SubstitutionMap get(GenericSignature genericSig,
                             SubstitutionMap substitutions);

  /// General form that takes two callbacks.
  static SubstitutionMap get(GenericSignature genericSig,
                             TypeSubstitutionFn subs,
                             LookupConformanceFn lookupConformance);

  /// Takes an array of replacement types already in the correct form, together
  /// with a conformance lookup callback.
  static SubstitutionMap get(GenericSignature genericSig,
                             ArrayRef<Type> replacementTypes,
                             LookupConformanceFn lookupConformance);

  /// Build a substitution map from the substitutions represented by
  /// the given in-flight substitution.
  ///
  /// This function should generally only be used by the substitution
  /// subsystem.
  static SubstitutionMap get(GenericSignature genericSig,
                             ArrayRef<Type> replacementTypes,
                             InFlightSubstitution &IFS);

  /// Build a substitution map from the substitutions represented by
  /// the given in-flight substitution.
  ///
  /// This function should generally only be used by the substitution
  /// subsystem.
  static SubstitutionMap get(GenericSignature genericSig,
                             InFlightSubstitution &IFS);

  /// Retrieve the generic signature describing the environment in which
  /// substitutions occur.
  GenericSignature getGenericSignature() const;

  /// Retrieve the array of protocol conformances, which line up with the
  /// requirements of the generic signature.
  ArrayRef<ProtocolConformanceRef> getConformances() const;

  /// Look up a conformance for the given type to the given protocol.
  ProtocolConformanceRef lookupConformance(CanType type,
                                           ProtocolDecl *proto) const;

  /// Whether the substitution map is empty.
  bool empty() const;

  /// Whether the substitution has any substitutable parameters, i.e.,
  /// it is non-empty and at least one of the type parameters can be
  /// substituted (i.e., is not mapped to a concrete type).
  bool hasAnySubstitutableParams() const;
  
  /// True if this substitution map is an identity mapping.
  bool isIdentity() const;

  /// Whether the substitution map is non-empty.
  explicit operator bool() const { return !empty(); }

  /// Retrieve the array of replacement types, which line up with the
  /// generic parameters.
  ArrayRef<Type> getReplacementTypes() const;

  /// Retrieve the array of replacement types for the innermost generic
  /// parameters.
  ArrayRef<Type> getInnermostReplacementTypes() const;

  RecursiveTypeProperties getRecursiveProperties() const;

  /// Whether the replacement types are all canonical.
  bool isCanonical() const;

  /// Return the canonical form of this substitution map.
  SubstitutionMap getCanonical(bool canonicalizeSignature = true) const;

  /// Apply a substitution to all replacement types in the map. Does not
  /// change keys.
  SubstitutionMap subst(SubstitutionMap subMap,
                        SubstOptions options = std::nullopt) const;

  /// Apply a substitution to all replacement types in the map. Does not
  /// change keys.
  SubstitutionMap subst(TypeSubstitutionFn subs,
                        LookupConformanceFn conformances,
                        SubstOptions options = std::nullopt) const;

  /// Apply an in-flight substitution to all replacement types in the map.
  /// Does not change keys.
  ///
  /// This should generally not be used outside of the substitution
  /// subsystem.
  SubstitutionMap subst(InFlightSubstitution &subs) const;

  /// Create a substitution map for a protocol conformance.
  static SubstitutionMap
  getProtocolSubstitutions(ProtocolConformanceRef conformance);

  /// Create a substitution map for a protocol conformance.
  static SubstitutionMap
  getProtocolSubstitutions(ProtocolDecl *protocol,
                           Type selfType,
                           ProtocolConformanceRef conformance);

  /// Given that 'derivedDecl' is an override of 'baseDecl' in a subclass,
  /// and 'derivedSubs' is a set of substitutions written in terms of the
  /// generic signature of 'derivedDecl', produce a set of substitutions
  /// written in terms of the generic signature of 'baseDecl'.
  static SubstitutionMap
  getOverrideSubstitutions(const ValueDecl *baseDecl,
                           const ValueDecl *derivedDecl);

  /// Variant of the above for when we have the generic signatures but not
  /// the decls for 'derived' and 'base'.
  static SubstitutionMap
  getOverrideSubstitutions(const NominalTypeDecl *baseNominal,
                           const NominalTypeDecl *derivedNominal,
                           GenericSignature baseSig,
                           const GenericParamList *derivedParams);

  /// Swap archetypes in the substitution map's replacement types with their
  /// interface types.
  SubstitutionMap mapReplacementTypesOutOfContext() const;

  /// Verify that the conformances stored in this substitution map match the
  /// replacement types provided.
  void verify(bool allowInvalid=true) const;

  /// Whether to dump the full substitution map, or just a minimal useful subset
  /// (on a single line).
  enum class DumpStyle { Minimal, Full };
  /// Dump the contents of this substitution map for debugging purposes.
  void dump(toolchain::raw_ostream &out, DumpStyle style = DumpStyle::Full,
            unsigned indent = 0) const;

  LANGUAGE_DEBUG_DUMP;

  /// Profile the substitution map, for use with LLVM's FoldingSet.
  void profile(toolchain::FoldingSetNodeID &id) const;

  const void *getOpaqueValue() const { return storage; }

  static SubstitutionMap getFromOpaqueValue(const void *ptr) {
    return SubstitutionMap(const_cast<Storage *>((const Storage *)ptr));
  }

  static SubstitutionMap getEmptyKey() {
    return SubstitutionMap(
             (Storage *)toolchain::DenseMapInfo<void*>::getEmptyKey());
  }

  static SubstitutionMap getTombstoneKey() {
    return SubstitutionMap(
               (Storage *)toolchain::DenseMapInfo<void*>::getTombstoneKey());
  }

  friend bool operator ==(SubstitutionMap lhs, SubstitutionMap rhs) {
    return lhs.storage == rhs.storage;
  }

  friend bool operator !=(SubstitutionMap lhs, SubstitutionMap rhs) {
    return lhs.storage != rhs.storage;
  }

private:
  friend class GenericSignature;
  friend class GenericEnvironment;
  friend struct QuerySubstitutionMap;

  /// Look up the replacement for the given type parameter or interface type.
  /// Note that this only finds replacements for maps that are directly
  /// stored inside the map. In most cases, you should call Type::subst()
  /// instead, since that will resolve member types also.
  Type lookupSubstitution(GenericTypeParamType *type) const;
};

inline toolchain::raw_ostream &operator<<(toolchain::raw_ostream &OS,
                                     const SubstitutionMap &subs) {
  subs.dump(OS);
  return OS;
}

/// A function object suitable for use as a \c TypeSubstitutionFn that
/// queries an array of replacement types.
struct QueryReplacementTypeArray {
  GenericSignature sig;
  ArrayRef<Type> types;

  Type operator()(SubstitutableType *type) const;
};

/// A function object suitable for use as a \c TypeSubstitutionFn that
/// queries an underlying \c SubstitutionMap.
struct QuerySubstitutionMap {
  SubstitutionMap subMap;

  Type operator()(SubstitutableType *type) const;
};

/// Functor class suitable for use as a \c LookupConformanceFn to look up a
/// conformance in a \c SubstitutionMap.
class LookUpConformanceInSubstitutionMap {
  SubstitutionMap Subs;
public:
  explicit LookUpConformanceInSubstitutionMap(SubstitutionMap Subs)
    : Subs(Subs) {}

  ProtocolConformanceRef operator()(InFlightSubstitution &IFS,
                                    Type dependentType,
                                    ProtocolDecl *proto) const;
};

struct OverrideSubsInfo {
  unsigned BaseDepth;
  unsigned OrigDepth;
  SubstitutionMap BaseSubMap;
  const GenericParamList *DerivedParams;

  OverrideSubsInfo(const NominalTypeDecl *baseNominal,
                   const NominalTypeDecl *derivedNominal,
                   GenericSignature baseSig,
                   const GenericParamList *derivedParams);
};

struct QueryOverrideSubs {
  OverrideSubsInfo info;

  explicit QueryOverrideSubs(const OverrideSubsInfo &info)
    : info(info) {}

  Type operator()(SubstitutableType *type) const;
};

struct LookUpConformanceInOverrideSubs {
  OverrideSubsInfo info;

  explicit LookUpConformanceInOverrideSubs(const OverrideSubsInfo &info)
    : info(info) {}

  ProtocolConformanceRef operator()(InFlightSubstitution &IFS,
                                    Type dependentType,
                                    ProtocolDecl *proto) const;
};

// Substitute the outer generic parameters from a substitution map, ignoring
/// inner generic parameters with a given depth.
struct OuterSubstitutions {
  SubstitutionMap subs;
  unsigned depth;

  Type operator()(SubstitutableType *type) const;
  ProtocolConformanceRef operator()(InFlightSubstitution &IFS,
                                    Type dependentType,
                                    ProtocolDecl *proto) const;
};

} // end namespace language

namespace toolchain {
  template <>
  struct PointerLikeTypeTraits<language::SubstitutionMap> {
    static void *getAsVoidPointer(language::SubstitutionMap map) {
      return const_cast<void *>(map.getOpaqueValue());
    }
    static language::SubstitutionMap getFromVoidPointer(const void *ptr) {
      return language::SubstitutionMap::getFromOpaqueValue(ptr);
    }

    /// Note: Assuming storage is at least 4-byte aligned.
    enum { NumLowBitsAvailable = 2 };
  };

  // Substitution maps hash just like pointers.
  template<> struct DenseMapInfo<language::SubstitutionMap> {
    static language::SubstitutionMap getEmptyKey() {
      return language::SubstitutionMap::getEmptyKey();
    }
    static language::SubstitutionMap getTombstoneKey() {
      return language::SubstitutionMap::getTombstoneKey();
    }
    static unsigned getHashValue(language::SubstitutionMap map) {
      return DenseMapInfo<void*>::getHashValue(map.getOpaqueValue());
    }
    static bool isEqual(language::SubstitutionMap lhs,
                        language::SubstitutionMap rhs) {
      return lhs.getOpaqueValue() == rhs.getOpaqueValue();
    }
  };

}

#endif
