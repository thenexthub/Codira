//===--- GenericSignature.h - Generic Signature AST -------------*- C++ -*-===//
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
// This file defines the GenericSignature class and its related classes.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_AST_GENERIC_SIGNATURE_H
#define LANGUAGE_AST_GENERIC_SIGNATURE_H

#include "language/AST/PrintOptions.h"
#include "language/AST/Requirement.h"
#include "language/AST/Type.h"
#include "language/AST/TypeAlignments.h"
#include "language/Basic/Debug.h"
#include "toolchain/ADT/ArrayRef.h"
#include "toolchain/ADT/FoldingSet.h"
#include "toolchain/ADT/SmallVector.h"
#include "toolchain/Support/TrailingObjects.h"
#include <utility>

namespace language {

class ProtocolConformanceRef;
class ProtocolType;
class SubstitutionMap;
class GenericEnvironment;
class GenericTypeParamType;

namespace rewriting {
  class RequirementMachine;
}

/// An access path used to find a particular protocol conformance within
/// a generic signature.
///
/// One can follow a conformance path to extract any conformance that is
/// derivable within the generic signature. For example, given:
///
/// \code
///   fn f<C: Collection>(_: C) where C.Iterator.Element: Hashable { }
/// \endcode
///
/// One can extract conformances for various types and protocols, including
/// those written directly (\c C: Collection, \c C.Iterator.Element: Hashable),
/// and others that can be derived (\c C: Sequence,
/// \c C.Iterator: IteratorProtocol, \c C.Iterator.Element: Equatable).
///
/// A conformance access path is a sequence of (dependent type, protocol decl)
/// pairs that starts at an explicit requirement in the generic signature
/// (e.g., \c C: Collection). Each subsequent step names a dependent
/// type and protocol that refers to an explicit requirement in the requirement
/// signature of the previous step's protocol. For example, consider the
/// derived conformance \c C.Iterator: IteratorProtocol, which has the
/// following path:
///
/// \code
/// (C, Collection) -> (Self, Sequence) -> (Self.Iterator, IteratorProtocol)
/// \endcode
///
/// Therefore, the path starts at \c C: Collection. It then retrieves the
/// \c Sequence conformance of \c C (because \c Collection inherits
/// \c Sequence). Finally, it extracts the conformance of the associated type
/// \c Iterator to \c IteratorProtocol from the \c Sequence protocol.
class ConformancePath {
public:
  /// An entry in the conformance access path, which is described by the
  /// dependent type on which the conformance is stated as the protocol to
  /// which.
  typedef std::pair<Type, ProtocolDecl *> Entry;

private:
  ArrayRef<Entry> path;

  ConformancePath(ArrayRef<Entry> path) : path(path) {}

  friend class GenericSignatureImpl;
  friend class rewriting::RequirementMachine;

public:
  typedef const Entry *const_iterator;
  typedef const_iterator iterator;

  unsigned size() const { return path.size(); }
  const_iterator begin() const { return path.begin(); }
  const_iterator end() const { return path.end(); }

  const Entry &back() const { return path.back(); }

  void print(raw_ostream &OS) const;

  LANGUAGE_DEBUG_DUMP;
};

class CanGenericSignature;
class GenericSignatureImpl;
class GenericTypeParamType;

/// Describes the generic signature of a particular declaration, including
/// both the generic type parameters and the requirements placed on those
/// generic parameters.
class GenericSignature {
  const GenericSignatureImpl *Ptr;

public:
  /// Create a new generic signature with the given type parameters and
  /// requirements. The requirements must already be minimal and canonical;
  /// to build a signature from an arbitrary set of requirements, use
  /// language::buildGenericSignature() instead.
  static GenericSignature get(ArrayRef<GenericTypeParamType *> params,
                              ArrayRef<Requirement> requirements,
                              bool isKnownCanonical = false);

  /// Produce a new generic signature which drops all of the marker
  /// protocol conformance requirements associated with this one.
  GenericSignature withoutMarkerProtocols() const;

public:
  static ASTContext &getASTContext(ArrayRef<GenericTypeParamType *> params,
                                   ArrayRef<Requirement> requirements);

public:
  /*implicit*/ GenericSignature(const GenericSignatureImpl *P = 0) : Ptr(P) {}

  const GenericSignatureImpl *getPointer() const { return Ptr; }

  bool isNull() const { return Ptr == 0; }

  const GenericSignatureImpl *operator->() const {
    assert(Ptr && "Cannot dereference a null GenericSignature!");
    return Ptr;
  }

  explicit operator bool() const { return Ptr != 0; }

  friend toolchain::hash_code hash_value(GenericSignature sig) {
    using toolchain::hash_value;
    return hash_value(sig.getPointer());
  }

  void print(raw_ostream &OS,
             const PrintOptions &Options = PrintOptions()) const;
  void print(ASTPrinter &Printer,
             const PrintOptions &Opts = PrintOptions()) const;
  LANGUAGE_DEBUG_DUMP;
  std::string getAsString() const;

  /// Returns the canonical generic signature, or \c nullptr if the underlying
  /// pointer is \c nullptr. The result is cached.
  CanGenericSignature getCanonicalSignature() const;

  // Support for FoldingSet.
  void Profile(toolchain::FoldingSetNodeID &id) const;

  static void Profile(toolchain::FoldingSetNodeID &ID,
                      ArrayRef<GenericTypeParamType *> genericParams,
                      ArrayRef<Requirement> requirements);
public:
  using RequiredProtocols = SmallVector<ProtocolDecl *, 2>;

  /// Stores a set of requirements on a type parameter. Used by
  /// GenericEnvironment for building archetypes.
  struct LocalRequirements {
    Type superclass;

    RequiredProtocols protos;
    LayoutConstraint layout;

    Type packShape;
  };

private:
  // Direct comparison is disabled for generic signatures.  Canonicalize them
  // first, or use isEqual.
  void operator==(GenericSignature T) const = delete;
  void operator!=(GenericSignature T) const = delete;

public:
  /// Retrieve the generic parameters.
  ArrayRef<GenericTypeParamType *> getGenericParams() const;

  /// Retrieve the innermost generic parameters.
  ///
  /// Given a generic signature for a nested generic type, produce an
  /// array of the generic parameters for the innermost generic type.
  ArrayRef<GenericTypeParamType *> getInnermostGenericParams() const;

  /// Returns the depth that a generic parameter at the next level of
  /// nesting would have. This is zero for the empty signature,
  /// and one plus the depth of the final generic parameter otherwise.
  unsigned getNextDepth() const;

  /// Retrieve the requirements.
  ArrayRef<Requirement> getRequirements() const;

  /// Returns the generic environment that provides fresh contextual types
  /// (archetypes) that correspond to the interface types in this generic
  /// signature.
  GenericEnvironment *getGenericEnvironment() const;

  /// Return the requirements of this generic signature that are not also
  /// satisfied by \c otherSig.
  ///
  /// \param otherSig Another generic signature whose generic parameters are
  /// equivalent to or a subset of the generic parameters in this signature.
  SmallVector<Requirement, 4>
  requirementsNotSatisfiedBy(GenericSignature otherSig) const;

  /// Return the reduced version of the given type under this generic
  /// signature.
  CanType getReducedType(Type type) const;

  /// Check invariants.
  void verify() const;

  /// Check invariants for a list of requirements that are understood to
  /// be valid in the given signature; used to verify a protocol's
  /// requirement signature against the protocol generic signature
  /// <Self where Self : P>.
  void verify(ArrayRef<Requirement> reqts) const;
};

/// A reference to a canonical generic signature.
class CanGenericSignature : public GenericSignature {
  bool isActuallyCanonicalOrNull() const;

public:
  /// Create a new generic signature with the given type parameters and
  /// requirements, first canonicalizing the types.
  static CanGenericSignature
  getCanonical(ArrayRef<GenericTypeParamType *> params,
               ArrayRef<Requirement> requirements);

public:
  CanGenericSignature(std::nullptr_t) : GenericSignature(nullptr) {}

  explicit CanGenericSignature(const GenericSignatureImpl *P = 0)
      : GenericSignature(P) {
    assert(isActuallyCanonicalOrNull() &&
           "Forming a CanGenericSignature out of a non-canonical signature!");
  }
  explicit CanGenericSignature(GenericSignature S) : GenericSignature(S) {
    assert(isActuallyCanonicalOrNull() &&
           "Forming a CanGenericSignature out of a non-canonical signature!");
  }

  ArrayRef<CanTypeWrapper<GenericTypeParamType>> getGenericParams() const;

  bool operator==(CanGenericSignature S) const {
    return getPointer() == S.getPointer();
  }
  bool operator!=(CanGenericSignature S) const { return !operator==(S); }
};

/// The underlying implementation of generic signatures.
class alignas(1 << TypeAlignInBits) GenericSignatureImpl final
  : public toolchain::FoldingSetNode,
    private toolchain::TrailingObjects<GenericSignatureImpl, GenericTypeParamType *,
                                  Requirement> {
  friend class ASTContext;
  friend GenericSignature;
  friend TrailingObjects;

  GenericSignatureImpl(const GenericSignatureImpl&) = delete;
  void operator=(const GenericSignatureImpl&) = delete;

  const unsigned NumGenericParams;
  const unsigned NumRequirements;

  GenericEnvironment *GenericEnv = nullptr;

  rewriting::RequirementMachine *Machine = nullptr;

  // Make vanilla new/delete illegal.
  void *operator new(size_t Bytes) = delete;
  void operator delete(void *Data) = delete;

  size_t numTrailingObjects(OverloadToken<GenericTypeParamType *>) const {
    return NumGenericParams;
  }
  size_t numTrailingObjects(OverloadToken<Requirement>) const {
    return NumRequirements;
  }

  GenericSignatureImpl(ArrayRef<GenericTypeParamType *> params,
                       ArrayRef<Requirement> requirements,
                       bool isKnownCanonical);

  // FIXME: Making this a CanGenericSignature reveals callers are violating
  // the interface's invariants.
  mutable toolchain::PointerUnion<const GenericSignatureImpl *, ASTContext *>
    CanonicalSignatureOrASTContext;

  friend class ArchetypeType;

public:
  /// Only allow allocation by doing a placement new.
  void *operator new(size_t Bytes, void *Mem) {
    assert(Mem);
    return Mem;
  }

  /// Returns the depth of the last generic parameter.
  unsigned getMaxDepth() const;

  /// Transform the requirements into a form where implicit Copyable and
  /// Escapable conformances are omitted, and their absence is explicitly
  /// noted.
  void getRequirementsWithInverses(
      SmallVector<Requirement, 2> &reqs,
      SmallVector<InverseRequirement, 2> &inverses) const;

  /// Iterate over all generic parameters, passing a flag to the callback
  /// indicating if the generic parameter is canonical or not.
  void forEachParam(
    toolchain::function_ref<void(GenericTypeParamType *, bool)> callback) const;

  /// Check if the generic signature makes all generic parameters
  /// concrete.
  bool areAllParamsConcrete() const;

  /// Check if the generic signature has a parameter pack.
  bool hasParameterPack() const;

  /// Compute the number of conformance requirements in this signature.
  unsigned getNumConformanceRequirements() const {
    unsigned result = 0;
    for (const auto &req : getRequirements()) {
      if (req.getKind() == RequirementKind::Conformance)
        ++result;
    }

    return result;
  }

  /// Return true if these two generic signatures are equal.
  bool isEqual(GenericSignature Other) const;

  /// Determines whether this GenericSignature is canonical.
  bool isCanonical() const;
  
  ASTContext &getASTContext() const;

  /// Retrieve the requirement machine for the given generic signature.
  rewriting::RequirementMachine *getRequirementMachine() const;

  /// Collects a set of requirements on a type parameter. Used by
  /// GenericEnvironment for building archetypes.
  GenericSignature::LocalRequirements getLocalRequirements(Type depType) const;

  /// Uniquing for the ASTContext.
  void Profile(toolchain::FoldingSetNodeID &ID) const {
    Profile(ID, getGenericParams(), getRequirements());
  }
  
  /// Determine whether the given dependent type is required to be a class.
  bool requiresClass(Type type) const;

  /// Determine the superclass bound on the given dependent type.
  Type getSuperclassBound(Type type) const;

  /// Determine the set of protocols to which the given type parameter is
  /// required to conform.
  GenericSignature::RequiredProtocols getRequiredProtocols(Type type) const;

  /// Determine whether the given type parameter is required to conform to
  /// the given protocol.
  bool requiresProtocol(Type type, ProtocolDecl *proto) const;

  /// Determine whether a conformance requirement of the given type to the
  /// given protocol prohibits the use of an isolated conformance.
  ///
  /// The use of an isolated conformance to satisfy a requirement T: P is
  /// prohibited when T is a type parameter and T, or some type that can be
  /// used to reach T, also conforms to Sendable or SendableMetatype. In that
  /// case, the conforming type and the protocol (Sendable or SendableMetatype)
  /// is returned.
  ///
  /// If there is no such requirement, returns std::nullopt.
  std::optional<std::pair<Type, ProtocolDecl *>>
  prohibitsIsolatedConformance(Type type) const;

  /// Determine whether the given dependent type is equal to a concrete type.
  bool isConcreteType(Type type) const;

  /// Return the concrete type that the given type parameter is constrained to,
  /// or the null Type if it is not the subject of a concrete same-type
  /// constraint.
  Type getConcreteType(Type type) const;

  /// Return the layout constraint that the given type parameter is constrained
  /// to, or the null LayoutConstraint if it is not the subject of layout
  /// constraint.
  LayoutConstraint getLayoutConstraint(Type type) const;

  /// Return whether two type parameters represent the same type under this
  /// generic signature.
  ///
  /// The type parameters must be known to not be concrete within the context.
  bool areReducedTypeParametersEqual(Type type1, Type type2) const;

  /// Determine if \c sig can prove \c requirement, meaning that it can deduce
  /// T: Foo or T == U (etc.) with the information it knows. This includes
  /// checking against global state, if any/all of the types in the requirement
  /// are concrete, not type parameters.
  bool isRequirementSatisfied(
      Requirement requirement,
      bool allowMissing = false,
      bool brokenPackBehavior = false) const;

  bool isReducedType(Type type) const;

  /// Return the reduced version of the given type parameter under this generic
  /// signature. To reduce a type that more generally contains type parameters,
  /// use GenericSignature::getReducedType().
  CanType getReducedTypeParameter(CanType type) const;

  /// Determine whether the given type parameter is defined under this generic
  /// signature.
  bool isValidTypeParameter(Type type) const;

  /// Retrieve the conformance path used to extract the conformance of
  /// interface \c type to the given \c protocol.
  ///
  /// \param type The type parameter whose conformance path is to be
  /// queried.
  /// \param protocol A protocol to which \c type conforms.
  ///
  /// \returns the conformance path that starts at a requirement of
  /// this generic signature and ends at the conformance that makes \c type
  /// conform to \c protocol.
  ///
  /// \seealso ConformancePath
  ConformancePath getConformancePath(Type type,
                                     ProtocolDecl *protocol) const;

  /// Lookup a nested type with the given name within this type parameter.
  TypeDecl *lookupNestedType(Type type, Identifier name) const;

  /// Returns the shape equivalence class of the given type parameter.
  ///
  /// \param type The type parameter to compute the reduced shape for.
  /// Only type parameter packs have a shape, including dependent members
  /// whose root generic parameter is a pack.
  Type getReducedShape(Type type) const;

  /// Returns \c true if the given type parameter packs are in
  /// the same shape equivalence class.
  bool haveSameShape(Type type1, Type type2) const;

  /// Returns all unique shape classes defined by this generic signature.
  SmallVector<CanType, 2> getShapeClasses() const;

  /// Get the ordinal of a generic parameter in this generic signature.
  ///
  /// For example, if you have a generic signature for a nested context like:
  ///   <t_0_0, t_0_1, t_1_0>
  /// then this will return 0 for t_0_0, 1 for t_0_1, and 2 for t_1_0.
  unsigned getGenericParamOrdinal(GenericTypeParamType *param) const;

  /// Get a substitution map that maps all of the generic signature's
  /// generic parameters to themselves.
  SubstitutionMap getIdentitySubstitutionMap() const;

  /// Get the sugared form of a generic parameter type.
  GenericTypeParamType *getSugaredType(GenericTypeParamType *type) const;

  /// Get the sugared form of a type by substituting any
  /// generic parameter types by their sugared form.
  Type getSugaredType(Type type) const;

  /// Given a type parameter, compute the most specific supertype (upper bound),
  /// possibly dependent on other type parameters.
  ///
  ///
  /// \param forExistentialSelf If true, we ensure the result does not include
  /// any type parameters rooted in the same generic parameter as the one given.
  ///
  /// \param includeParameterizedProtocols If true, we form parameterized
  /// protocol types if we find that the given type's primary associated types
  /// are sufficiently constrained.
  ///
  /// \note If the upper bound is a protocol or protocol composition,
  /// will return an instance of \c ExistentialType.
  Type getUpperBound(Type type,
                     bool forExistentialSelf,
                     bool includeParameterizedProtocols) const;

  /// Utility wrapper for use when this is an opened existential signature.
  Type getExistentialType(Type type) const;

  static void Profile(toolchain::FoldingSetNodeID &ID,
                      ArrayRef<GenericTypeParamType *> genericParams,
                      ArrayRef<Requirement> requirements);
  
  void print(raw_ostream &OS, const PrintOptions &Options = PrintOptions()) const;
  void print(ASTPrinter &Printer, const PrintOptions &Opts = PrintOptions()) const;
  LANGUAGE_DEBUG_DUMP;
  std::string getAsString() const;

private:
  friend GenericSignature;
  friend CanGenericSignature;

  /// Retrieve the generic parameters.
  ArrayRef<GenericTypeParamType *> getGenericParams() const {
    return ArrayRef<GenericTypeParamType *>(
        {getTrailingObjects<GenericTypeParamType *>(), NumGenericParams});
  }

  /// Retrieve the innermost generic parameters.
  ///
  /// Given a generic signature for a nested generic type, produce an
  /// array of the generic parameters for the innermost generic type.
  ArrayRef<GenericTypeParamType *> getInnermostGenericParams() const;

  /// Retrieve the requirements.
  ArrayRef<Requirement> getRequirements() const {
    return {getTrailingObjects<Requirement>(), NumRequirements};
  }

  /// Returns the canonical generic signature. The result is cached.
  CanGenericSignature getCanonicalSignature() const;

  /// Returns the generic environment that provides fresh contextual types
  /// (archetypes) that correspond to the interface types in this generic
  /// signature.
  GenericEnvironment *getGenericEnvironment() const;

  /// Return the requirements of this generic signature that are not also
  /// satisfied by \c otherSig.
  ///
  /// \param otherSig Another generic signature whose generic parameters are
  /// equivalent to or a subset of the generic parameters in this signature.
  SmallVector<Requirement, 4>
  requirementsNotSatisfiedBy(GenericSignature otherSig) const;

  /// Return the reduced version of the given type under this generic
  /// signature.
  CanType getReducedType(Type type) const;
};

void simple_display(raw_ostream &out, GenericSignature sig);

inline bool CanGenericSignature::isActuallyCanonicalOrNull() const {
  return getPointer() == nullptr ||
         getPointer() ==
             toolchain::DenseMapInfo<GenericSignatureImpl *>::getEmptyKey() ||
         getPointer() ==
             toolchain::DenseMapInfo<GenericSignatureImpl *>::getTombstoneKey() ||
         getPointer()->isCanonical();
}

int compareAssociatedTypes(AssociatedTypeDecl *assocType1,
                           AssociatedTypeDecl *assocType2);

int compareDependentTypes(Type type1, Type type2);

/// Verify the correctness of the given generic signature.
///
/// This routine will test that the given generic signature is both minimal
/// and canonical, emitting errors if it is not.
void validateGenericSignature(ASTContext &context,
                              GenericSignature sig);

/// Verify all of the generic signatures in the given module.
void validateGenericSignaturesInModule(ModuleDecl *module);

/// Build a generic signature from the given requirements, which are not
/// required to be minimal or canonical, and may contain unresolved
/// DependentMemberTypes.
///
/// \param baseSignature if non-null, the new parameters and requirements
///// are added on; existing requirements of the base signature might become
///// redundant. Otherwise if null, build a new signature from scratch.
/// \param allowInverses if true, default requirements to Copyable/Escapable are
/// expanded for generic parameters.
GenericSignature buildGenericSignature(
    ASTContext &ctx,
    GenericSignature baseSignature,
    SmallVector<GenericTypeParamType *, 2> addedParameters,
    SmallVector<Requirement, 2> addedRequirements,
    bool allowInverses);

/// Summary of error conditions detected by the Requirement Machine.
enum class GenericSignatureErrorFlags {
  /// The original requirements referenced a non-existent type parameter,
  /// or the original requirements were in conflict with each other, meaning
  /// there are no possible concrete substitutions which satisfy the
  /// generic signature.
  HasInvalidRequirements = (1<<0),

  /// The generic signature had non-redundant concrete conformance
  /// requirements, which means the rewrite system used for minimization
  /// must be discarded and a new one built for queries.
  HasConcreteConformances = (1<<1),

  /// The Knuth-Bendix completion procedure failed to construct a confluent
  /// rewrite system.
  CompletionFailed = (1<<2)
};

using GenericSignatureErrors = OptionSet<GenericSignatureErrorFlags>;

/// AbstractGenericSignatureRequest and InferredGenericSignatureRequest
/// return this type, which stores a GenericSignature together with the
/// above set of error flags.
using GenericSignatureWithError = toolchain::PointerIntPair<GenericSignature, 3,
                                                       GenericSignatureErrors>;

/// Build a generic signature from the given requirements, which are not
/// required to be minimal or canonical, and may contain unresolved
/// DependentMemberTypes. The generic signature is returned with the
/// error flags (if any) that were raised while building the signature.
///
/// \param baseSignature if non-null, the new parameters and requirements
///// are added on; existing requirements of the base signature might become
///// redundant. Otherwise if null, build a new signature from scratch.
/// \param allowInverses if true, default requirements to Copyable/Escapable are
/// expanded for generic parameters.
GenericSignatureWithError buildGenericSignatureWithError(
    ASTContext &ctx,
    GenericSignature baseSignature,
    SmallVector<GenericTypeParamType *, 2> addedParameters,
    SmallVector<Requirement, 2> addedRequirements,
    bool allowInverses);

} // end namespace language

namespace toolchain {
static inline raw_ostream &operator<<(raw_ostream &OS,
                                      language::GenericSignature Sig) {
  Sig.print(OS);
  return OS;
}

// A GenericSignature casts like a GenericSignatureImpl*.
template <> struct simplify_type<const ::language::GenericSignature> {
  typedef const ::language::GenericSignatureImpl *SimpleType;
  static SimpleType getSimplifiedValue(const ::language::GenericSignature &Val) {
    return Val.getPointer();
  }
};
template <>
struct simplify_type<::language::GenericSignature>
    : public simplify_type<const ::language::GenericSignature> {};

template <> struct DenseMapInfo<language::GenericSignature> {
  static language::GenericSignature getEmptyKey() {
    return toolchain::DenseMapInfo<language::GenericSignatureImpl *>::getEmptyKey();
  }
  static language::GenericSignature getTombstoneKey() {
    return toolchain::DenseMapInfo<language::GenericSignatureImpl *>::getTombstoneKey();
  }
  static unsigned getHashValue(language::GenericSignature Val) {
    return DenseMapInfo<language::GenericSignatureImpl *>::getHashValue(
        Val.getPointer());
  }
  static bool isEqual(language::GenericSignature LHS,
                      language::GenericSignature RHS) {
    return LHS.getPointer() == RHS.getPointer();
  }
};

// A GenericSignature is "pointer like".
template <> struct PointerLikeTypeTraits<language::GenericSignature> {
public:
  static inline language::GenericSignature getFromVoidPointer(void *P) {
    return (language::GenericSignatureImpl *)P;
  }
  static inline void *getAsVoidPointer(language::GenericSignature S) {
    return (void *)S.getPointer();
  }
  enum { NumLowBitsAvailable = language::TypeAlignInBits };
};

template <> struct PointerLikeTypeTraits<language::CanGenericSignature> {
public:
  static inline language::CanGenericSignature getFromVoidPointer(void *P) {
    return language::CanGenericSignature((language::GenericSignatureImpl *)P);
  }
  static inline void *getAsVoidPointer(language::CanGenericSignature S) {
    return (void *)S.getPointer();
  }
  enum { NumLowBitsAvailable = language::TypeAlignInBits };
};
} // end namespace toolchain

#endif // LANGUAGE_AST_GENERIC_SIGNATURE_H
