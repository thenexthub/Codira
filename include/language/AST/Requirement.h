//===--- Requirement.h - Codira Requirement ASTs -----------------*- C++ -*-===//
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
// This file defines the Requirement class and subclasses.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_AST_REQUIREMENT_H
#define LANGUAGE_AST_REQUIREMENT_H

#include "language/AST/LayoutConstraint.h"
#include "language/AST/RequirementKind.h"
#include "language/AST/KnownProtocols.h"
#include "language/AST/Type.h"
#include "language/Basic/Debug.h"
#include "toolchain/ADT/Hashing.h"
#include "toolchain/ADT/PointerIntPair.h"
#include "toolchain/Support/ErrorHandling.h"

namespace language {
class GenericContext;

/// Return type of Requirement::checkRequirement().
enum class CheckRequirementResult : uint8_t {
  /// The requirement was fully satisfied.
  Success,

  /// The subject type conforms conditionally; the sub-requirements are
  /// conditional requirements which must be checked.
  ConditionalConformance,

  /// The subject type is a pack type; the sub-requirements are the
  /// element-wise requirements which must be checked.
  PackRequirement,

  /// The requirement cannot ever be satisfied.
  RequirementFailure,

  /// Some other requirement is expected to fail, or there was an invalid
  /// conformance and an error should be diagnosed elsewhere, so this
  /// requirement does not need to be diagnosed.
  SubstitutionFailure
};

/// A single requirement placed on the type parameters (or associated
/// types thereof) of a
class Requirement {
  toolchain::PointerIntPair<Type, 3, RequirementKind> FirstTypeAndKind;
  /// The second element of the requirement. Its content is dependent
  /// on the requirement kind.
  /// The payload of the following enum should always match the kind!
  /// Any access to the fields of this enum should first check if the
  /// requested access matches the kind of the requirement.
  union {
    Type SecondType;
    LayoutConstraint SecondLayout;
  };

public:
  /// Create a conformance or same-type requirement.
  Requirement(RequirementKind kind, Type first, Type second)
      : FirstTypeAndKind(first, kind), SecondType(second) {
    assert(first);
    assert(second);
    assert(kind != RequirementKind::Layout);
  }

  /// Create a layout constraint requirement.
  Requirement(RequirementKind kind, Type first, LayoutConstraint second)
      : FirstTypeAndKind(first, kind), SecondLayout(second) {
    assert(first);
    assert(second);
    assert(kind == RequirementKind::Layout);
  }


  /// Determine the kind of requirement.
  RequirementKind getKind() const { return FirstTypeAndKind.getInt(); }

  /// Retrieve the first type.
  Type getFirstType() const {
    return FirstTypeAndKind.getPointer();
  }

  /// Retrieve the second type.
  Type getSecondType() const {
    assert(getKind() != RequirementKind::Layout);
    return SecondType;
  }

  /// Retrieve the layout constraint.
  LayoutConstraint getLayoutConstraint() const {
    assert(getKind() == RequirementKind::Layout);
    return SecondLayout;
  }

  friend toolchain::hash_code hash_value(const Requirement &requirement) {
    using toolchain::hash_value;

    toolchain::hash_code first =
        hash_value(requirement.FirstTypeAndKind.getOpaqueValue());
    toolchain::hash_code second;
    switch (requirement.getKind()) {
    case RequirementKind::SameShape:
    case RequirementKind::Conformance:
    case RequirementKind::Superclass:
    case RequirementKind::SameType:
      second = hash_value(requirement.getSecondType());
      break;

    case RequirementKind::Layout:
      second = hash_value(requirement.getLayoutConstraint());
      break;
    }

    return toolchain::hash_combine(first, second);
  }

  friend bool operator==(const Requirement &lhs,
                         const Requirement &rhs) {
    if (lhs.FirstTypeAndKind.getOpaqueValue()
          != rhs.FirstTypeAndKind.getOpaqueValue())
      return false;

    switch (lhs.getKind()) {
    case RequirementKind::SameShape:
    case RequirementKind::Conformance:
    case RequirementKind::Superclass:
    case RequirementKind::SameType:
      return lhs.getSecondType().getPointer() ==
          rhs.getSecondType().getPointer();

    case RequirementKind::Layout:
      return lhs.getLayoutConstraint() == rhs.getLayoutConstraint();
    }
    toolchain_unreachable("Unhandled RequirementKind in switch");
  }

  friend bool operator!=(const Requirement &lhs,
                         const Requirement &rhs) {
    return !(lhs == rhs);
  }

  /// Whether this requirement's types contain ErrorTypes.
  bool hasError() const;

  /// Whether this requirement is written with canonical types.
  bool isCanonical() const;

  /// Canonicalize the types in this requirement.
  Requirement getCanonical() const;

  /// Subst the types involved in this requirement.
  ///
  /// The \c args arguments are passed through to Type::subst.
  template <typename ...Args>
  Requirement subst(Args &&...args) const {
    auto newFirst = getFirstType().subst(std::forward<Args>(args)...);
    switch (getKind()) {
    case RequirementKind::SameShape:
    case RequirementKind::Conformance:
    case RequirementKind::Superclass:
    case RequirementKind::SameType: {
      auto newSecond = getSecondType().subst(std::forward<Args>(args)...);
      return Requirement(getKind(), newFirst, newSecond);
    }
    case RequirementKind::Layout:
      return Requirement(getKind(), newFirst, getLayoutConstraint());
    }

    toolchain_unreachable("Unhandled RequirementKind in switch.");
  }

  ProtocolDecl *getProtocolDecl() const;

  /// Determines if this substituted requirement is satisfied.
  ///
  /// \param subReqs An out parameter initialized to a list of simpler
  /// requirements which the caller must check to ensure this
  /// requirement is completely satisfied.
  /// \param isolatedConformances If non-NULL, will be provided with all of the
  /// isolated conformances that
  CheckRequirementResult checkRequirement(
      SmallVectorImpl<Requirement> &subReqs,
      bool allowMissing = false,
      SmallVectorImpl<ProtocolConformanceRef> *isolatedConformances = nullptr
  ) const;

  /// Determines if this substituted requirement can ever be satisfied,
  /// possibly with additional substitutions.
  ///
  /// For example, if 'T' is unconstrained, then a superclass requirement
  /// 'T : C' can be satisfied; however, if 'T' already has an unrelated
  /// superclass requirement, 'T : C' cannot be satisfied.
  bool canBeSatisfied() const;
  
  /// True if the requirement states a conformance to an invertible protocol
  /// that is implied by default (such as `Copyable` or `Escapable`.
  bool isInvertibleProtocolRequirement() const;

  /// Linear order on requirements in a generic signature.
  int compare(const Requirement &other) const;

  LANGUAGE_DEBUG_DUMP;
  void dump(raw_ostream &out) const;
  void print(raw_ostream &os, const PrintOptions &opts) const;
  void print(ASTPrinter &printer, const PrintOptions &opts) const;
};

inline void simple_display(toolchain::raw_ostream &out, const Requirement &req) {
  req.print(out, PrintOptions());
}

enum class CheckRequirementsResult : uint8_t {
  Success,

  /// One of the requirements was unsatisfied.
  RequirementFailure,

  /// One of the requirements contained error types, either because of an
  /// invalid conformance or because it contained a member type that was
  /// dependent on an earlier conformance requirement that failed.
  SubstitutionFailure
};

/// Check if each substituted requirement is satisfied. The requirement must
/// not contain any type parameters.
CheckRequirementsResult checkRequirements(ArrayRef<Requirement> requirements);

/// Check if each substituted requirement is satisfied. If the requirement
/// contains type parameters, and the answer would depend on the context of
/// those type parameters, then `nullopt` is returned.
std::optional<CheckRequirementsResult>
checkRequirementsWithoutContext(ArrayRef<Requirement> requirements);

/// Check if each requirement is satisfied after applying the given
/// substitutions. The substitutions must replace all type parameters that
/// appear in the requirement with concrete types or archetypes.
CheckRequirementsResult checkRequirements(ArrayRef<Requirement> requirements,
                                          TypeSubstitutionFn substitutions,
                                          SubstOptions options = std::nullopt);

/// A requirement as written in source, together with a source location. See
/// ProtocolDecl::getStructuralRequirements().
struct StructuralRequirement {
  /// A requirement with resolved in the structural resolution stage.
  Requirement req;

  /// The source location where the requirement is written, for diagnostics.
  SourceLoc loc;
};

/// An "anti-conformance" requirement `Subject: ~Protocol`.
struct InverseRequirement {
  Type subject;
  ProtocolDecl *protocol;
  SourceLoc loc;

  InverseRequirement(Type subject, ProtocolDecl *protocol, SourceLoc loc);

  InvertibleProtocolKind getKind() const;

  /// Linear order on inverse requirements in a generic signature.
  int compare(const InverseRequirement &other) const;

  /// Appends additional requirements corresponding to defaults for the given
  /// generic parameters.
  static void expandDefaults(ASTContext &ctx,
                             ArrayRef<Type> gps,
                             SmallVectorImpl<StructuralRequirement> &result);

  void print(raw_ostream &os, const PrintOptions &opts, bool forInherited=false) const;
};

} // end namespace language

#endif
