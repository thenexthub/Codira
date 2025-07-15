//===--- AvailabilitySpec.cpp - Codira Availability Query ASTs -------------===//
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
// This file implements the availability specification AST classes.
//
//===----------------------------------------------------------------------===//

#include "language/AST/AvailabilitySpec.h"
#include "language/AST/ASTContext.h"
#include "language/AST/AvailabilityDomain.h"
#include "language/AST/DiagnosticsSema.h"
#include "language/AST/TypeCheckRequests.h"
#include "toolchain/Support/raw_ostream.h"

using namespace language;

AvailabilitySpec *AvailabilitySpec::createWildcard(ASTContext &ctx,
                                                   SourceLoc starLoc) {
  return new (ctx) AvailabilitySpec(AvailabilityDomain::forUniversal(), starLoc,
                                    /*Version=*/{},
                                    /*VersionStartLoc=*/{});
}

static SourceRange getSpecSourceRange(SourceLoc domainLoc,
                                      SourceRange versionRange) {
  if (domainLoc.isInvalid())
    return SourceRange();

  if (versionRange.isValid())
    return SourceRange(domainLoc, versionRange.End);

  return SourceRange(domainLoc);
}

AvailabilitySpec *AvailabilitySpec::createForDomain(ASTContext &ctx,
                                                    AvailabilityDomain domain,
                                                    SourceLoc loc,
                                                    toolchain::VersionTuple version,
                                                    SourceRange versionRange) {
  return new (ctx)
      AvailabilitySpec(domain, getSpecSourceRange(loc, versionRange), version,
                       versionRange.Start);
}

AvailabilitySpec *AvailabilitySpec::createForDomainIdentifier(
    ASTContext &ctx, Identifier domainIdentifier, SourceLoc loc,
    toolchain::VersionTuple version, SourceRange versionRange) {
  return new (ctx)
      AvailabilitySpec(domainIdentifier, getSpecSourceRange(loc, versionRange),
                       version, versionRange.Start);
}

AvailabilitySpec *AvailabilitySpec::clone(ASTContext &ctx) const {
  return new (ctx) AvailabilitySpec(getDomainOrIdentifier().copy(ctx), SrcRange,
                                    Version, VersionStartLoc);
}

void AvailabilitySpec::print(toolchain::raw_ostream &os) const {
  getDomainOrIdentifier().print(os);

  if (!getRawVersion().empty())
    os << " " << getRawVersion().getAsString();
}

std::optional<AvailabilityDomain>
AvailabilitySpec::resolveInDeclContext(const DeclContext *declContext) {
  auto domainOrIdentifier = getDomainOrIdentifier();
  auto result =
      domainOrIdentifier.resolveInDeclContext(getStartLoc(), declContext);
  DomainOrIdentifier = domainOrIdentifier;
  return result;
}

std::optional<SemanticAvailabilitySpec>
AvailabilitySpec::getSemanticAvailabilitySpec(
    const DeclContext *declContext) const {
  return evaluateOrDefault(declContext->getASTContext().evaluator,
                           SemanticAvailabilitySpecRequest{this, declContext},
                           std::nullopt);
}

toolchain::VersionTuple SemanticAvailabilitySpec::getVersion() const {
  // For macOS Big Sur, we canonicalize 10.16 to 11.0 for compile-time
  // checking since clang canonicalizes availability markup. However, to
  // support Beta versions of macOS Big Sur where the OS
  // reports 10.16 at run time, we need to compare against 10.16,
  //
  // This means for:
  //
  // if #available(macOS 10.16, *) { ... }
  //
  // we need to store the uncanonicalized version for codegen and canonicalize
  // it as necessary for compile-time checks.
  return canonicalizePlatformVersion(getDomain().getPlatformKind(),
                                     spec->getRawVersion());
}

std::optional<SemanticAvailabilitySpec>
SemanticAvailabilitySpecs::Filter::operator()(
    const AvailabilitySpec *spec) const {
  if (auto semanticSpec = spec->getSemanticAvailabilitySpec(declContext))
    return semanticSpec;
  return std::nullopt;
}

std::optional<SemanticAvailabilitySpec>
SemanticAvailabilitySpecRequest::evaluate(
    Evaluator &evaluator, const AvailabilitySpec *spec,
    const DeclContext *declContext) const {
  if (spec->isInvalid())
    return std::nullopt;

  auto &diags = declContext->getASTContext().Diags;
  AvailabilitySpec *mutableSpec = const_cast<AvailabilitySpec *>(spec);
  if (!mutableSpec->resolveInDeclContext(declContext).has_value())
    return std::nullopt;

  auto version = spec->getRawVersion();
  if (!VersionRange::isValidVersion(version)) {
    diags
        .diagnose(spec->getStartLoc(),
                  diag::availability_unsupported_version_number, version)
        .highlight(spec->getVersionSrcRange());
    return std::nullopt;
  }

  return SemanticAvailabilitySpec(spec);
}

std::optional<std::optional<SemanticAvailabilitySpec>>
SemanticAvailabilitySpecRequest::getCachedResult() const {
  auto *spec = std::get<0>(getStorage());
  if (spec->isInvalid())
    return std::optional<SemanticAvailabilitySpec>();

  auto domainOrIdentifier = spec->getDomainOrIdentifier();
  if (!domainOrIdentifier.isResolved())
    return std::nullopt;

  return SemanticAvailabilitySpec(spec);
}

void SemanticAvailabilitySpecRequest::cacheResult(
    std::optional<SemanticAvailabilitySpec> value) const {
  auto *mutableSpec = const_cast<AvailabilitySpec *>(std::get<0>(getStorage()));
  if (!value)
    mutableSpec->setInvalid();
}
