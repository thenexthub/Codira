//===--- AvailabilityContext.cpp - Codira Availability Structures ----------===//
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

#include "language/AST/AvailabilityContext.h"
#include "language/AST/ASTContext.h"
#include "language/AST/AvailabilityConstraint.h"
#include "language/AST/AvailabilityContextStorage.h"
#include "language/AST/AvailabilityInference.h"
#include "language/AST/AvailabilityScope.h"
#include "language/AST/Decl.h"
#include "language/AST/Module.h"
#include "language/Basic/Assertions.h"

using namespace language;

struct AvailabilityDomainInfoComparator {
  bool operator()(const AvailabilityContext::DomainInfo &lhs,
                  const AvailabilityContext::DomainInfo &rhs) const {
    StableAvailabilityDomainComparator domainComparator;
    return domainComparator(lhs.getDomain(), rhs.getDomain());
  }
};

static bool constrainBool(bool &existing, bool other) {
  if (existing || !other)
    return false;
  existing = other;
  return true;
}

static bool constrainRange(AvailabilityRange &existing,
                           const AvailabilityRange &other) {
  if (!other.isContainedIn(existing))
    return false;

  existing = other;
  return true;
}

/// Returns true if `domainInfos` will be constrained by merging the domain
/// availability represented by `otherDomainInfo`. Additionally, this function
/// has a couple of side-effects:
///
///   - If any existing domain availability ought to be constrained by
///     `otherDomainInfo` then that value will be updated in place.
///   - If any existing value in `domainInfos` should be replaced when
///     `otherDomainInfo` is added, then that existing value is removed
///     and `otherDomainInfo` is appended to `domainInfosToAdd`.
///
static bool constrainDomainInfos(
    AvailabilityContext::DomainInfo otherDomainInfo,
    toolchain::SmallVectorImpl<AvailabilityContext::DomainInfo> &domainInfos,
    toolchain::SmallVectorImpl<AvailabilityContext::DomainInfo> &domainInfosToAdd) {
  bool isConstrained = false;
  bool shouldAdd = true;
  auto otherDomain = otherDomainInfo.getDomain();
  auto end = domainInfos.rend();

  // Iterate over domainInfos in reverse order to allow items to be removed
  // during iteration.
  for (auto iter = domainInfos.rbegin(); iter != end; ++iter) {
    auto &domainInfo = *iter;
    auto domain = domainInfo.getDomain();

    // We found an existing available range for the domain. Constrain it if
    // necessary.
    if (domain == otherDomain) {
      shouldAdd = false;
      isConstrained |= domainInfo.constrainRange(otherDomainInfo.getRange());
      continue;
    }

    // Check whether an existing unavailable domain contains the domain that
    // would be added. If so, there's nothing to do because the availability of
    // the domain is already as constrained as it can be.
    if (domainInfo.isUnavailable() && domain.contains(otherDomain)) {
      DEBUG_ASSERT(!isConstrained);
      return false;
    }

    // If the domain that will be added is unavailable, check whether the
    // existing domain is contained within it. If it is, availability for the
    // existing domain should be removed because it has been superseded.
    if (otherDomainInfo.isUnavailable() && otherDomain.contains(domain)) {
      domainInfos.erase((iter + 1).base());
      isConstrained = true;
    }
  }

  // If the new domain availability isn't already covered by an item in
  // `domainInfos`, then it needs to be added. Defer adding the new domain
  // availability until later when the entire set of domain infos can be
  // re-sorted once.
  if (shouldAdd) {
    domainInfosToAdd.push_back(otherDomainInfo);
    return true;
  }

  return isConstrained;
}

/// Constrains `domainInfos` by merging them with `otherDomainInfos`. Returns
/// true if any changes were made to `domainInfos`.
static bool constrainDomainInfos(
    toolchain::SmallVectorImpl<AvailabilityContext::DomainInfo> &domainInfos,
    toolchain::ArrayRef<AvailabilityContext::DomainInfo> otherDomainInfos) {
  bool isConstrained = false;
  toolchain::SmallVector<AvailabilityContext::DomainInfo, 4> domainInfosToAdd;
  for (auto otherDomainInfo : otherDomainInfos) {
    isConstrained |=
        constrainDomainInfos(otherDomainInfo, domainInfos, domainInfosToAdd);
  }

  if (!isConstrained)
    return false;

  // Add the new domains and then re-sort.
  for (auto domainInfo : domainInfosToAdd)
    domainInfos.push_back(domainInfo);

  toolchain::sort(domainInfos, AvailabilityDomainInfoComparator());
  return true;
}

bool AvailabilityContext::DomainInfo::constrainRange(
    const AvailabilityRange &otherRange) {
  return ::constrainRange(range, otherRange);
}

AvailabilityContext
AvailabilityContext::forPlatformRange(const AvailabilityRange &range,
                                      const ASTContext &ctx) {
  return AvailabilityContext(
      Storage::get(range, /*isDeprecated=*/false, /*domainInfos=*/{}, ctx));
}

AvailabilityContext
AvailabilityContext::forInliningTarget(const ASTContext &ctx) {
  return AvailabilityContext::forPlatformRange(
      AvailabilityRange::forInliningTarget(ctx), ctx);
}

AvailabilityContext
AvailabilityContext::forDeploymentTarget(const ASTContext &ctx) {
  return AvailabilityContext::forPlatformRange(
      AvailabilityRange::forDeploymentTarget(ctx), ctx);
}

AvailabilityContext
AvailabilityContext::forLocation(SourceLoc loc, const DeclContext *declContext,
                                 const AvailabilityScope **refinedScope) {
  auto &ctx = declContext->getASTContext();
  SourceFile *sf =
      loc.isValid()
          ? declContext->getParentModule()->getSourceFileContainingLocation(loc)
          : declContext->getParentSourceFile();

  // If our source location is invalid (this may be synthesized code), climb the
  // decl context hierarchy until we find a location that is valid, merging
  // availability contexts on the way up.
  //
  // Because we are traversing decl contexts, we will miss availability scopes
  // in synthesized code that are introduced by AST elements that are themselves
  // not decl contexts, such as `#available(..)` and property declarations.
  // Therefore a reference with an invalid source location that is contained
  // inside an `#available()` and with no intermediate decl context will not be
  // refined. For now, this is fine, but if we ever synthesize #available(),
  // this could become a problem..

  // We can assume we are running on at least the minimum inlining target.
  auto baseAvailability = AvailabilityContext::forInliningTarget(ctx);
  auto isInvalidLoc = [sf](SourceLoc loc) {
    return sf ? loc.isInvalid() : true;
  };
  while (declContext && isInvalidLoc(loc)) {
    const Decl *decl = declContext->getInnermostDeclarationDeclContext();
    if (!decl)
      break;

    baseAvailability.constrainWithDecl(decl);
    loc = decl->getLoc();
    declContext = decl->getDeclContext();
  }

  if (!sf || loc.isInvalid())
    return baseAvailability;

  auto *rootScope = AvailabilityScope::getOrBuildForSourceFile(*sf);
  if (!rootScope)
    return baseAvailability;

  auto *scope = rootScope->findMostRefinedSubContext(loc, ctx);
  if (!scope)
    return baseAvailability;

  if (refinedScope)
    *refinedScope = scope;

  auto availability = scope->getAvailabilityContext();
  availability.constrainWithContext(baseAvailability, ctx);
  return availability;
}

AvailabilityContext AvailabilityContext::forDeclSignature(const Decl *decl) {
  return forLocation(decl->getLoc(), decl->getInnermostDeclContext());
}

AvailabilityContext
AvailabilityContext::forAlwaysAvailable(const ASTContext &ctx) {
  return AvailabilityContext(Storage::get(AvailabilityRange::alwaysAvailable(),
                                          /*isDeprecated=*/false,
                                          /*domainInfos=*/{}, ctx));
}

AvailabilityRange AvailabilityContext::getPlatformRange() const {
  return storage->platformRange;
}

std::optional<AvailabilityRange>
AvailabilityContext::getAvailabilityRange(AvailabilityDomain domain,
                                          const ASTContext &ctx) const {
  DEBUG_ASSERT(domain.supportsContextRefinement());

  if (domain.isActivePlatform(ctx))
    return storage->platformRange;

  for (auto domainInfo : storage->getDomainInfos()) {
    if (domain == domainInfo.getDomain() && !domainInfo.isUnavailable())
      return domainInfo.getRange();
  }

  return std::nullopt;
}

bool AvailabilityContext::isUnavailable() const {
  for (auto domainInfo : storage->getDomainInfos()) {
    if (domainInfo.isUnavailable())
      return true;
  }
  return false;
}

bool AvailabilityContext::containsUnavailableDomain(
    AvailabilityDomain domain) const {
  for (auto domainInfo : storage->getDomainInfos()) {
    if (domainInfo.isUnavailable()) {
      if (domainInfo.getDomain().contains(domain))
        return true;
    }
  }
  return false;
}

bool AvailabilityContext::isDeprecated() const { return storage->isDeprecated; }

void AvailabilityContext::constrainWithContext(const AvailabilityContext &other,
                                               const ASTContext &ctx) {
  bool isConstrained = false;
  auto platformRange = storage->platformRange;
  isConstrained |= constrainRange(platformRange, other.storage->platformRange);

  bool isDeprecated = storage->isDeprecated;
  isConstrained |= constrainBool(isDeprecated, other.storage->isDeprecated);

  auto domainInfos = storage->copyDomainInfos();
  isConstrained |=
      constrainDomainInfos(domainInfos, other.storage->getDomainInfos());

  if (!isConstrained)
    return;

  storage = Storage::get(platformRange, isDeprecated, domainInfos, ctx);
}

void AvailabilityContext::constrainWithPlatformRange(
    const AvailabilityRange &range, const ASTContext &ctx) {
  auto platformRange = storage->platformRange;
  if (!constrainRange(platformRange, range))
    return;

  storage = Storage::get(platformRange, storage->isDeprecated,
                         storage->getDomainInfos(), ctx);
}

void AvailabilityContext::constrainWithAvailabilityRange(
    const AvailabilityRange &range, AvailabilityDomain domain,
    const ASTContext &ctx) {
  DEBUG_ASSERT(domain.supportsContextRefinement());

  if (domain.isActivePlatform(ctx)) {
    constrainWithPlatformRange(range, ctx);
    return;
  }

  auto domainInfos = storage->copyDomainInfos();
  if (!constrainDomainInfos(domainInfos, {DomainInfo(domain, range)}))
    return;

  storage = Storage::get(storage->platformRange, storage->isDeprecated,
                         domainInfos, ctx);
}

void AvailabilityContext::constrainWithUnavailableDomain(
    AvailabilityDomain domain, const ASTContext &ctx) {
  auto domainInfos = storage->copyDomainInfos();
  if (!constrainDomainInfos(domainInfos, {DomainInfo::unavailable(domain)}))
    return;

  storage = Storage::get(storage->platformRange, storage->isDeprecated,
                         domainInfos, ctx);
}

void AvailabilityContext::constrainWithDecl(const Decl *decl) {
  constrainWithDeclAndPlatformRange(decl, AvailabilityRange::alwaysAvailable());
}

void AvailabilityContext::constrainWithDeclAndPlatformRange(
    const Decl *decl, const AvailabilityRange &otherPlatformRange) {
  auto &ctx = decl->getASTContext();
  bool isConstrained = false;
  auto platformRange = storage->platformRange;
  isConstrained |= constrainRange(platformRange, otherPlatformRange);

  bool isDeprecated = storage->isDeprecated;
  isConstrained |= constrainBool(isDeprecated, decl->isDeprecated());

  // Compute the availability constraints for the decl when used in this context
  // and then map those constraints to domain infos. The result will be merged
  // into the existing domain infos for this context.
  toolchain::SmallVector<DomainInfo, 4> declDomainInfos;
  AvailabilityConstraintFlags flags =
      AvailabilityConstraintFlag::SkipEnclosingExtension;
  auto constraints =
      language::getAvailabilityConstraintsForDecl(decl, *this, flags);
  for (auto constraint : constraints) {
    auto attr = constraint.getAttr();
    auto domain = attr.getDomain();
    switch (constraint.getReason()) {
    case AvailabilityConstraint::Reason::UnconditionallyUnavailable:
    case AvailabilityConstraint::Reason::Obsoleted:
    case AvailabilityConstraint::Reason::UnavailableForDeployment:
      declDomainInfos.push_back(DomainInfo::unavailable(domain));
      break;
    case AvailabilityConstraint::Reason::PotentiallyUnavailable:
      if (auto introducedRange = attr.getIntroducedRange(ctx)) {
        if (domain.isActivePlatform(ctx)) {
          isConstrained |= constrainRange(platformRange, *introducedRange);
        } else {
          declDomainInfos.push_back({domain, *introducedRange});
        }
      }
      break;
    }
  }

  auto domainInfos = storage->copyDomainInfos();
  isConstrained |= constrainDomainInfos(domainInfos, declDomainInfos);

  if (!isConstrained)
    return;

  storage = Storage::get(platformRange, isDeprecated, domainInfos,
                         decl->getASTContext());
}

bool AvailabilityContext::isContainedIn(const AvailabilityContext other) const {
  // The available versions range be the same or smaller.
  if (!storage->platformRange.isContainedIn(other.storage->platformRange))
    return false;

  // The set of deprecated domains should be the same or larger.
  if (!storage->isDeprecated && other.storage->isDeprecated)
    return false;

  // Every unavailable domain in the other context should be contained in some
  // unavailable domain in this context.
  bool disjointUnavailability = toolchain::any_of(
      other.storage->getDomainInfos(), [&](const DomainInfo &otherDomainInfo) {
        return toolchain::none_of(storage->getDomainInfos(),
                             [&otherDomainInfo](const DomainInfo &domainInfo) {
                               return domainInfo.getDomain().contains(
                                   otherDomainInfo.getDomain());
                             });
      });
  if (disjointUnavailability)
    return false;

  return true;
}

static std::string
stringForAvailability(const AvailabilityRange &availability) {
  if (availability.isAlwaysAvailable())
    return "all";
  if (availability.isKnownUnreachable())
    return "none";

  return availability.getVersionString();
}

void AvailabilityContext::print(toolchain::raw_ostream &os) const {
  os << "version=" << stringForAvailability(getPlatformRange());

  auto domainInfos = storage->getDomainInfos();
  if (!domainInfos.empty()) {
    auto availableInfos = toolchain::make_filter_range(
        domainInfos, [](auto info) { return !info.isUnavailable(); });

    if (!availableInfos.empty()) {
      os << " available=";
      toolchain::interleave(
          availableInfos, os,
          [&](const DomainInfo &domainInfo) {
            domainInfo.getDomain().print(os);
            if (domainInfo.getDomain().isVersioned() &&
                domainInfo.getRange().hasMinimumVersion())
              os << ">=" << domainInfo.getRange().getAsString();
          },
          ",");
    }

    auto unavailableInfos = toolchain::make_filter_range(
        domainInfos, [](auto info) { return info.isUnavailable(); });

    if (!unavailableInfos.empty()) {
      os << " unavailable=";
      toolchain::interleave(
          unavailableInfos, os,
          [&](const DomainInfo &domainInfo) {
            domainInfo.getDomain().print(os);
          },
          ",");
    }
  }

  if (isDeprecated())
    os << " deprecated";
}

void AvailabilityContext::dump() const { print(toolchain::errs()); }

bool verifyDomainInfos(
    toolchain::ArrayRef<AvailabilityContext::DomainInfo> domainInfos) {
  // Checks that the following invariants hold:
  //   - The domain infos are sorted using AvailabilityDomainInfoComparator.
  //   - There is not more than one info per-domain.
  if (domainInfos.empty())
    return true;

  AvailabilityDomainInfoComparator compare;
  auto prev = domainInfos.begin();
  auto next = prev;
  auto end = domainInfos.end();
  for (++next; next != end; prev = next, ++next) {
    const auto &prevInfo = *prev;
    const auto &nextInfo = *next;

    if (compare(nextInfo, prevInfo))
      return false;

    // Since the infos are sorted by domain, infos with the same domain should
    // be adjacent.
    if (prevInfo.getDomain() == nextInfo.getDomain())
      return false;
  }

  return true;
}

bool AvailabilityContext::verify(const ASTContext &ctx) const {
  return verifyDomainInfos(storage->getDomainInfos());
}

void AvailabilityContext::Storage::Profile(
    toolchain::FoldingSetNodeID &ID, const AvailabilityRange &platformRange,
    bool isDeprecated,
    toolchain::ArrayRef<AvailabilityContext::DomainInfo> domainInfos) {
  platformRange.getRawVersionRange().Profile(ID);
  ID.AddBoolean(isDeprecated);
  ID.AddInteger(domainInfos.size());
  for (auto domainInfo : domainInfos) {
    domainInfo.Profile(ID);
  }
}
