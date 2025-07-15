//===--- AvailabilityConstraint.cpp - Codira Availability Constraints ------===//
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

#include "language/AST/AvailabilityConstraint.h"
#include "language/AST/ASTContext.h"
#include "language/AST/AvailabilityContext.h"
#include "language/AST/AvailabilityInference.h"
#include "language/AST/Decl.h"

using namespace language;

AvailabilityDomainAndRange
AvailabilityConstraint::getDomainAndRange(const ASTContext &ctx) const {
  switch (getReason()) {
  case Reason::UnconditionallyUnavailable:
    // Technically, unconditional unavailability doesn't have an associated
    // range. However, if you view it as a special case of obsoletion, then an
    // unconditionally unavailable declaration is "always obsoleted."
    return AvailabilityDomainAndRange(getDomain().getRemappedDomain(ctx),
                                      AvailabilityRange::alwaysAvailable());
  case Reason::Obsoleted:
    return getAttr().getObsoletedDomainAndRange(ctx).value();
  case Reason::UnavailableForDeployment:
  case Reason::PotentiallyUnavailable:
    return getAttr().getIntroducedDomainAndRange(ctx).value();
  }
}

bool AvailabilityConstraint::isActiveForRuntimeQueries(
    const ASTContext &ctx) const {
  if (getAttr().getPlatform() == PlatformKind::none)
    return true;

  return language::isPlatformActive(getAttr().getPlatform(), ctx.LangOpts,
                                 /*forTargetVariant=*/false,
                                 /*forRuntimeQuery=*/true);
}

static bool constraintIsStronger(const AvailabilityConstraint &lhs,
                                 const AvailabilityConstraint &rhs) {
  DEBUG_ASSERT(lhs.getDomain() == rhs.getDomain());

  // If the constraints have matching domains but different reasons, the
  // constraint with the lowest reason is "strongest".
  if (lhs.getReason() != rhs.getReason())
    return lhs.getReason() < rhs.getReason();

  switch (lhs.getReason()) {
  case AvailabilityConstraint::Reason::UnconditionallyUnavailable:
    // Just keep the first.
    return false;

  case AvailabilityConstraint::Reason::Obsoleted:
    // Pick the larger obsoleted range.
    return *lhs.getAttr().getObsoleted() < *rhs.getAttr().getObsoleted();

  case AvailabilityConstraint::Reason::UnavailableForDeployment:
  case AvailabilityConstraint::Reason::PotentiallyUnavailable:
    // Pick the smaller introduced range.
    return *lhs.getAttr().getIntroduced() > *rhs.getAttr().getIntroduced();
  }
}

void addConstraint(toolchain::SmallVector<AvailabilityConstraint, 4> &constraints,
                   const AvailabilityConstraint &constraint,
                   const ASTContext &ctx) {

  auto iter = toolchain::find_if(
      constraints, [&constraint](AvailabilityConstraint &existing) {
        return constraint.getDomain() == existing.getDomain();
      });

  // There's no existing constraint for the same domain so just add it.
  if (iter == constraints.end()) {
    constraints.emplace_back(constraint);
    return;
  }

  if (constraintIsStronger(constraint, *iter)) {
    constraints.erase(iter);
    constraints.emplace_back(constraint);
  }
}

std::optional<AvailabilityConstraint>
DeclAvailabilityConstraints::getPrimaryConstraint() const {
  std::optional<AvailabilityConstraint> result;

  auto isStrongerConstraint = [](const AvailabilityConstraint &lhs,
                                 const AvailabilityConstraint &rhs) {
    // Constraint reasons are defined in descending order of strength.
    if (lhs.getReason() != rhs.getReason())
      return lhs.getReason() < rhs.getReason();

    // Pick the constraint from the broader domain.
    if (lhs.getDomain() != rhs.getDomain())
      return rhs.getDomain().contains(lhs.getDomain());
    
    return false;
  };

  // Pick the strongest constraint.
  for (auto const &constraint : constraints) {
    if (!result || isStrongerConstraint(constraint, *result))
      result.emplace(constraint);
  }

  return result;
}

static bool canIgnoreConstraintInUnavailableContexts(
    const Decl *decl, const AvailabilityConstraint &constraint) {
  auto domain = constraint.getDomain();

  switch (constraint.getReason()) {
  case AvailabilityConstraint::Reason::UnconditionallyUnavailable:
    // Always reject uses of universally unavailable declarations, regardless
    // of context, since there are no possible compilation configurations in
    // which they are available. However, make an exception for types and
    // conformances, which can sometimes be awkward to avoid references to.
    if (!isa<TypeDecl>(decl) && !isa<ExtensionDecl>(decl)) {
      if (domain.isUniversal() || domain.isCodiraLanguage())
        return false;
    }
    return true;

  case AvailabilityConstraint::Reason::PotentiallyUnavailable:
    switch (domain.getKind()) {
    case AvailabilityDomain::Kind::Universal:
    case AvailabilityDomain::Kind::CodiraLanguage:
    case AvailabilityDomain::Kind::PackageDescription:
    case AvailabilityDomain::Kind::Embedded:
    case AvailabilityDomain::Kind::Custom:
      return false;
    case AvailabilityDomain::Kind::Platform:
      // Platform availability only applies to the target triple that the
      // binary is being compiled for. Since the same declaration can be
      // potentially unavailable from a given context when compiling for one
      // platform, but available from that context when compiling for a
      // different platform, it is overly strict to enforce potential platform
      // unavailability constraints in contexts that are unavailable to that
      // platform.
      return true;
    }
    return constraint.getDomain().isPlatform();

  case AvailabilityConstraint::Reason::Obsoleted:
  case AvailabilityConstraint::Reason::UnavailableForDeployment:
    return false;
  }
}

static bool
shouldIgnoreConstraintInContext(const Decl *decl,
                                const AvailabilityConstraint &constraint,
                                const AvailabilityContext &context) {
  if (!context.isUnavailable())
    return false;

  if (!canIgnoreConstraintInUnavailableContexts(decl, constraint))
    return false;

  return context.containsUnavailableDomain(constraint.getDomain());
}

/// Returns the `AvailabilityConstraint` that describes how \p attr restricts
/// use of \p decl in \p context or `std::nullopt` if there is no restriction.
static std::optional<AvailabilityConstraint>
getAvailabilityConstraintForAttr(const Decl *decl,
                                 const SemanticAvailableAttr &attr,
                                 const AvailabilityContext &context) {
  // Is the decl unconditionally unavailable?
  if (attr.isUnconditionallyUnavailable())
    return AvailabilityConstraint::unconditionallyUnavailable(attr);

  auto &ctx = decl->getASTContext();
  auto domain = attr.getDomain();
  auto deploymentRange = domain.getDeploymentRange(ctx);

  // Is the decl obsoleted in the deployment context?
  if (auto obsoletedRange = attr.getObsoletedRange(ctx)) {
    if (deploymentRange && deploymentRange->isContainedIn(*obsoletedRange))
      return AvailabilityConstraint::obsoleted(attr);
  }

  // Is the decl not yet introduced in the local context?
  if (auto introducedRange = attr.getIntroducedRange(ctx)) {
    if (domain.supportsContextRefinement()) {
      auto availableRange = context.getAvailabilityRange(domain, ctx);
      if (!availableRange || !availableRange->isContainedIn(*introducedRange))
        return AvailabilityConstraint::potentiallyUnavailable(attr);

      return std::nullopt;
    }

    // Is the decl not yet introduced in the deployment context?
    if (deploymentRange && !deploymentRange->isContainedIn(*introducedRange))
      return AvailabilityConstraint::unavailableForDeployment(attr);
  }

  // FIXME: [availability] Model deprecation as an availability constraint.
  return std::nullopt;
}

/// Returns the most specific platform domain from the availability attributes
/// attached to \p decl or `std::nullopt` if there are none. Platform specific
/// `@available` attributes for other platforms should be ignored. For example,
/// if a declaration has attributes for both iOS and macCatalyst, only the
/// macCatalyst attributes take effect when compiling for a macCatalyst target.
static std::optional<AvailabilityDomain>
activePlatformDomainForDecl(const Decl *decl) {
  std::optional<AvailabilityDomain> activeDomain;
  for (auto attr :
       decl->getSemanticAvailableAttrs(/*includingInactive=*/false)) {
    auto domain = attr.getDomain();
    if (!domain.isPlatform())
      continue;

    if (activeDomain && domain.contains(*activeDomain))
      continue;

    activeDomain.emplace(domain);
  }

  return activeDomain;
}

static void getAvailabilityConstraintsForDecl(
    toolchain::SmallVector<AvailabilityConstraint, 4> &constraints, const Decl *decl,
    const AvailabilityContext &context, AvailabilityConstraintFlags flags) {
  auto &ctx = decl->getASTContext();
  auto activePlatformDomain = activePlatformDomainForDecl(decl);
  bool includeAllDomains =
      flags.contains(AvailabilityConstraintFlag::IncludeAllDomains);

  for (auto attr : decl->getSemanticAvailableAttrs(includeAllDomains)) {
    auto domain = attr.getDomain();
    if (!includeAllDomains && domain.isPlatform() && activePlatformDomain &&
        !activePlatformDomain->contains(domain))
      continue;

    if (auto constraint = getAvailabilityConstraintForAttr(decl, attr, context))
      addConstraint(constraints, *constraint, ctx);
  }

  // After resolving constraints, remove any constraints that indicate the
  // declaration is unconditionally unavailable in a domain for which
  // the context is already unavailable.
  toolchain::erase_if(constraints, [&](const AvailabilityConstraint &constraint) {
    return shouldIgnoreConstraintInContext(decl, constraint, context);
  });
}

DeclAvailabilityConstraints
language::getAvailabilityConstraintsForDecl(const Decl *decl,
                                         const AvailabilityContext &context,
                                         AvailabilityConstraintFlags flags) {
  toolchain::SmallVector<AvailabilityConstraint, 4> constraints;

  // Generic parameters are always available.
  if (isa<GenericTypeParamDecl>(decl))
    return DeclAvailabilityConstraints();

  decl = decl->getAbstractSyntaxDeclForAttributes();

  getAvailabilityConstraintsForDecl(constraints, decl, context, flags);

  if (flags.contains(AvailabilityConstraintFlag::SkipEnclosingExtension))
    return constraints;

  // If decl is an extension member, query the attributes of the extension, too.
  //
  // Skip decls imported from Clang, though, as they could be associated to the
  // wrong extension and inherit unavailability incorrectly. ClangImporter
  // associates Objective-C protocol members to the first category where the
  // protocol is directly or indirectly adopted, no matter its availability
  // and the availability of other categories. rdar://problem/53956555
  if (decl->getClangNode())
    return constraints;

  auto parent = AvailabilityInference::parentDeclForInferredAvailability(decl);
  if (auto extension = dyn_cast_or_null<ExtensionDecl>(parent))
    getAvailabilityConstraintsForDecl(constraints, extension, context, flags);

  return constraints;
}
