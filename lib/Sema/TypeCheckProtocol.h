//===--- TypeCheckProtocol.h - Constraint-based Type Checking ----*- C++ -*-===//
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
#ifndef LANGUAGE_SEMA_PROTOCOL_H
#define LANGUAGE_SEMA_PROTOCOL_H

#include "TypeChecker.h"
#include "language/AST/AccessScope.h"
#include "language/AST/ASTContext.h"
#include "language/AST/RequirementEnvironment.h"
#include "language/AST/RequirementMatch.h"
#include "language/AST/Type.h"
#include "language/AST/Types.h"
#include "language/AST/Witness.h"
#include "language/Basic/Debug.h"
#include "language/Sema/ConstraintSystem.h"
#include "toolchain/ADT/ScopedHashTable.h"
#include "toolchain/ADT/SetVector.h"
#include "toolchain/ADT/SmallPtrSet.h"
#include "toolchain/ADT/SmallVector.h"

namespace language {

class AssociatedTypeDecl;
class AvailabilityRange;
class DeclContext;
class FuncDecl;
class NormalProtocolConformance;
class ProtocolDecl;
class TypeRepr;
class ValueDecl;

class RequirementEnvironment;

/// Gather the value witnesses for the given requirement.
///
/// \param DC A nominal type or extension context where the conformance
/// was declared.
/// \param req A member of a protocol that DC conforms to.
/// \param ignoringNames If non-null and there are no value
/// witnesses with the correct full name, the results will reflect
/// lookup for just the base name and the pointee will be set to
/// \c true.
SmallVector<ValueDecl *, 4> lookupValueWitnesses(DeclContext *DC,
                                                 ValueDecl *req,
                                                 bool *ignoringNames);

struct RequirementCheck;

class WitnessChecker {
public:
  using RequirementEnvironmentCacheKey =
      std::pair<const GenericSignatureImpl *, const ClassDecl *>;
  using RequirementEnvironmentCache =
      toolchain::DenseMap<RequirementEnvironmentCacheKey, RequirementEnvironment>;

protected:
  ASTContext &Context;
  ProtocolDecl *Proto;
  Type Adoptee;
  // The conforming context, either a nominal type or extension.
  DeclContext *DC;

  ASTContext &getASTContext() const { return Context; }

  RequirementEnvironmentCache ReqEnvironmentCache;

  WitnessChecker(ASTContext &ctx, ProtocolDecl *proto, Type adoptee,
                 DeclContext *dc);

  void lookupValueWitnessesViaImplementsAttr(ValueDecl *req,
                                             SmallVector<ValueDecl *, 4>
                                             &witnesses);

  bool findBestWitness(ValueDecl *requirement,
                       bool *ignoringNames,
                       NormalProtocolConformance *conformance,
                       SmallVectorImpl<RequirementMatch> &matches,
                       unsigned &numViable,
                       unsigned &bestIdx,
                       bool &doNotDiagnoseMatches);

  bool checkWitnessAvailability(ValueDecl *requirement,
                                ValueDecl *witness,
                                AvailabilityRange *requirementInfo);

  RequirementCheck checkWitness(ValueDecl *requirement,
                                const RequirementMatch &match);
};

/// The result of attempting to resolve a witness.
enum class ResolveWitnessResult {
  /// The resolution succeeded.
  Success,
  /// There was an explicit witness available, but it failed some
  /// criteria.
  ExplicitFailed,
  /// There was no witness available.
  Missing
};

/// The protocol conformance checker.
///
/// This helper class handles most of the details of checking whether a
/// given type (\c Adoptee) conforms to a protocol (\c Proto).
class ConformanceChecker : public WitnessChecker {
public:
  NormalProtocolConformance *Conformance;
  SourceLoc Loc;

  /// Record a (non-type) witness for the given requirement.
  void recordWitness(ValueDecl *requirement, const RequirementMatch &match);

  /// Record that the given optional requirement has no witness.
  void recordOptionalWitness(ValueDecl *requirement);

  /// Record that the given requirement has no valid witness.
  void recordInvalidWitness(ValueDecl *requirement);

  /// Check that the witness and requirement have compatible actor contexts.
  ///
  /// \param usesPreconcurrency Will be set true if the conformance is
  /// @preconcurrency and we made use of that fact.
  ///
  /// \returns the isolation that needs to be enforced to invoke the witness
  /// from the requirement, used when entering an actor-isolated synchronous
  /// witness from an asynchronous requirement.
  std::optional<ActorIsolation> checkActorIsolation(ValueDecl *requirement,
                                                    ValueDecl *witness,
                                                    bool &usesPreconcurrency);

  /// Enforce restrictions on non-final classes witnessing requirements
  /// involving the protocol 'Self' type.
  void checkNonFinalClassWitness(ValueDecl *requirement,
                                 ValueDecl *witness);

  /// Resolve a (non-type) witness via name lookup.
  ResolveWitnessResult resolveWitnessViaLookup(ValueDecl *requirement);

  /// Resolve a (non-type) witness via derivation.
  ResolveWitnessResult resolveWitnessViaDerivation(ValueDecl *requirement);

  /// Resolve a (non-type) witness via default definition or optional.
  ResolveWitnessResult resolveWitnessViaDefault(ValueDecl *requirement);

  /// Resolve a (non-type) witness by trying each standard strategy until one
  /// of them produces a result.
  ResolveWitnessResult
  resolveWitnessTryingAllStrategies(ValueDecl *requirement);

  ConformanceChecker(ASTContext &ctx, NormalProtocolConformance *conformance);

  ~ConformanceChecker();

  /// Resolve all of the non-type witnesses.
  void resolveValueWitnesses();

  /// Resolve the witness for the given non-type requirement as
  /// directly as possible, only resolving other witnesses if
  /// needed, e.g., to determine type witnesses used within the
  /// requirement.
  ///
  /// This entry point is designed to be used when the witness for a
  /// particular requirement and adoptee is required, before the
  /// conformance has been completed checked.
  void resolveSingleWitness(ValueDecl *requirement);
};

/// Match the given witness to the given requirement.
///
/// \returns the result of performing the match.
RequirementMatch matchWitness(
    DeclContext *dc, ValueDecl *req, ValueDecl *witness,
    toolchain::function_ref<
        std::tuple<std::optional<RequirementMatch>, Type, Type, Type, Type>(void)>
        setup,
    toolchain::function_ref<std::optional<RequirementMatch>(Type, Type)> matchTypes,
    toolchain::function_ref<RequirementMatch(bool, ArrayRef<OptionalAdjustment>)>
        finalize);

RequirementMatch
matchWitness(WitnessChecker::RequirementEnvironmentCache &reqEnvCache,
             ProtocolDecl *proto, RootProtocolConformance *conformance,
             DeclContext *dc, ValueDecl *req, ValueDecl *witness);

enum class TypeAdjustment : uint8_t {
  NoescapeToEscaping, NonsendableToSendable
};

/// Perform any necessary adjustments to the inferred associated type to
/// make it suitable for later use.
///
/// \param performed Will be set \c true if this operation performed
/// the adjustment, or \c false if the operation found a type that the
/// adjustment could have applied to but did not actually need to adjust it.
/// Unchanged otherwise.
Type adjustInferredAssociatedType(TypeAdjustment adjustment, Type type,
                                  bool &performed);

/// Find the @objc requirement that are witnessed by the given
/// declaration.
///
/// \param anySingleRequirement If true, returns at most a single requirement,
/// which might be any of the requirements that match.
///
/// \returns the set of requirements to which the given witness is a
/// witness.
toolchain::TinyPtrVector<ValueDecl *> findWitnessedObjCRequirements(
                                     const ValueDecl *witness,
                                     bool anySingleRequirement = false);

void diagnoseConformanceFailure(Type T,
                                ProtocolDecl *Proto,
                                DeclContext *DC,
                                SourceLoc ComplainLoc);

/// Find an associated type declaration that provides a default definition.
AssociatedTypeDecl *findDefaultedAssociatedType(
    DeclContext *dc, NominalTypeDecl *adoptee,
    AssociatedTypeDecl *assocType);

/// Determine whether this witness has an `@_implements` attribute whose
/// name matches that of the given requirement.
bool witnessHasImplementsAttrForRequiredName(ValueDecl *witness,
                                             ValueDecl *requirement);

/// Determine whether this witness has an `@_implements` attribute whose name
/// and protocol match that of the requirement exactly.
bool witnessHasImplementsAttrForExactRequirement(ValueDecl *witness,
                                                 ValueDecl *requirement);

using VisitedConformances = toolchain::SmallPtrSet<void *, 16>;

/// Visit each conformance within the given type.
///
/// If `body` returns true for any conformance, this function stops the
/// traversal and returns true.
bool forEachConformance(
    Type type, toolchain::function_ref<bool(ProtocolConformanceRef)> body,
    VisitedConformances *visitedConformances = nullptr);

/// Visit each conformance within the given conformance (including the given
/// one).
///
/// If `body` returns true for any conformance, this function stops the
/// traversal and returns true.
bool forEachConformance(
    ProtocolConformanceRef conformance,
    toolchain::function_ref<bool(ProtocolConformanceRef)> body,
    VisitedConformances *visitedConformances = nullptr);

/// Visit each conformance within the given substitution map.
///
/// If `body` returns true for any conformance, this function stops the
/// traversal and returns true.
bool forEachConformance(
    SubstitutionMap subs,
    toolchain::function_ref<bool(ProtocolConformanceRef)> body,
    VisitedConformances *visitedConformances = nullptr);

/// Visit each conformance within the given declaration reference.
///
/// If `body` returns true for any conformance, this function stops the
/// traversal and returns true.
bool forEachConformance(
    ConcreteDeclRef declRef,
    toolchain::function_ref<bool(ProtocolConformanceRef)> body,
    VisitedConformances *visitedConformances = nullptr);

}

#endif // LANGUAGE_SEMA_PROTOCOL_H
