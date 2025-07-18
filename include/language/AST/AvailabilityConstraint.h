//===--- AvailabilityConstraint.h - Codira Availability Constraints ------*-===//
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
// This file defines the AvailabilityConstraint class.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_AST_AVAILABILITY_CONSTRAINT_H
#define LANGUAGE_AST_AVAILABILITY_CONSTRAINT_H

#include "language/AST/Attr.h"
#include "language/AST/AvailabilityDomain.h"
#include "language/AST/AvailabilityRange.h"
#include "language/AST/PlatformKindUtils.h"
#include "language/Basic/Toolchain.h"
#include "language/Basic/OptionSet.h"

namespace language {

class ASTContext;
class AvailabilityContext;
class Decl;

/// Represents the reason a declaration could be considered unavailable in a
/// certain context.
class AvailabilityConstraint {
public:
  /// The reason that the availability constraint is unsatisfied.
  ///
  /// NOTE: The order of this enum matters. Reasons are defined in descending
  /// priority order.
  enum class Reason {
    /// The declaration is referenced in a context in which it is generally
    /// unavailable. For example, a reference to a declaration that is
    /// unavailable on macOS from a context that may execute on macOS has this
    /// constraint.
    UnconditionallyUnavailable,

    /// The declaration is referenced in a context in which it is considered
    /// obsolete. For example, a reference to a declaration that is obsolete in
    /// macOS 13 from a context that may execute on macOS 13 or later has this
    /// constraint.
    Obsoleted,

    /// The declaration is not available in the deployment configuration
    /// specified for this compilation. For example, the declaration might only
    /// be introduced in the Codira 6 language mode while the module is being
    /// compiled in the Codira 5 language mode. These availability constraints
    /// cannot be satisfied by adding constraining contextual availability using
    /// `@available` attributes or `if #available` queries.
    UnavailableForDeployment,

    /// The declaration is referenced in a context that does not have adequate
    /// availability constraints. For example, a reference to a declaration that
    /// was introduced in macOS 13 from a context that may execute on earlier
    /// versions of macOS cannot satisfy this constraint. The constraint
    /// can be satisfied, though, by introducing an `@available` attribute or an
    /// `if #available(...)` query.
    PotentiallyUnavailable,
  };

  /// Classifies constraints into different high level categories.
  enum class Kind {
    /// There are no contexts in which the declaration would be available.
    Unavailable,

    /// There are some contexts in which the declaration would be available if
    /// additional constraints were added.
    PotentiallyAvailable,
  };

private:
  toolchain::PointerIntPair<SemanticAvailableAttr, 2, Reason> attrAndReason;

  AvailabilityConstraint(Reason reason, SemanticAvailableAttr attr)
      : attrAndReason(attr, reason) {};

public:
  static AvailabilityConstraint
  unconditionallyUnavailable(SemanticAvailableAttr attr) {
    return AvailabilityConstraint(Reason::UnconditionallyUnavailable, attr);
  }

  static AvailabilityConstraint obsoleted(SemanticAvailableAttr attr) {
    return AvailabilityConstraint(Reason::Obsoleted, attr);
  }

  static AvailabilityConstraint
  unavailableForDeployment(SemanticAvailableAttr attr) {
    return AvailabilityConstraint(Reason::UnavailableForDeployment, attr);
  }

  static AvailabilityConstraint
  potentiallyUnavailable(SemanticAvailableAttr attr) {
    return AvailabilityConstraint(Reason::PotentiallyUnavailable, attr);
  }

  Reason getReason() const { return attrAndReason.getInt(); }
  SemanticAvailableAttr getAttr() const {
    return static_cast<SemanticAvailableAttr>(attrAndReason.getPointer());
  }

  Kind getKind() const {
    switch (getReason()) {
    case Reason::UnconditionallyUnavailable:
    case Reason::Obsoleted:
    case Reason::UnavailableForDeployment:
      return Kind::Unavailable;
    case Reason::PotentiallyUnavailable:
      return Kind::PotentiallyAvailable;
    }
  }

  /// Returns true if the constraint cannot be satisfied at runtime.
  bool isUnavailable() const { return getKind() == Kind::Unavailable; }

  /// Returns true if the constraint is unsatisfied but could be satisfied at
  /// runtime in a more constrained context.
  bool isPotentiallyAvailable() const {
    return getKind() == Kind::PotentiallyAvailable;
  }

  /// Returns the domain that the constraint applies to.
  AvailabilityDomain getDomain() const { return getAttr().getDomain(); }

  /// Returns the domain and range (remapped if necessary) in which the
  /// constraint must be satisfied. How the range should be interpreted depends
  /// on the reason for the constraint.
  AvailabilityDomainAndRange getDomainAndRange(const ASTContext &ctx) const;

  /// Some availability constraints are active for type-checking but cannot
  /// be translated directly into an `if #available(...)` runtime query.
  bool isActiveForRuntimeQueries(const ASTContext &ctx) const;
};

/// Represents a set of availability constraints that restrict use of a
/// declaration in a particular context. There can only be one active constraint
/// for a given `AvailabilityDomain`, but there may be multiple active
/// constraints from separate domains.
class DeclAvailabilityConstraints {
  using Storage = toolchain::SmallVector<AvailabilityConstraint, 4>;
  Storage constraints;

public:
  DeclAvailabilityConstraints() {}
  DeclAvailabilityConstraints(const Storage &&constraints)
      : constraints(constraints) {}

  /// Returns the strongest availability constraint or `std::nullopt` if empty.
  std::optional<AvailabilityConstraint> getPrimaryConstraint() const;

  using const_iterator = Storage::const_iterator;
  const_iterator begin() const { return constraints.begin(); }
  const_iterator end() const { return constraints.end(); }
};

enum class AvailabilityConstraintFlag : uint8_t {
  /// By default, the availability constraints for the members of extensions
  /// include the constraints for `@available` attributes that were written on
  /// the enclosing extension, since these members can be referred to without
  /// referencing the extension. When this flag is specified, though, only the
  /// attributes directly attached to the declaration are considered.
  SkipEnclosingExtension = 1 << 0,

  /// Include constraints for all domains, regardless of whether they are active
  /// or relevant to type checking.
  IncludeAllDomains = 1 << 1,
};
using AvailabilityConstraintFlags = OptionSet<AvailabilityConstraintFlag>;

/// Returns the set of availability constraints that restrict use of \p decl
/// when it is referenced from the given context. In other words, it is the
/// collection of of `@available` attributes with unsatisfied conditions.
DeclAvailabilityConstraints getAvailabilityConstraintsForDecl(
    const Decl *decl, const AvailabilityContext &context,
    AvailabilityConstraintFlags flags = std::nullopt);
} // end namespace language

#endif
