//===--- AvailabilityQuery.cpp - Codira Availability Queries ---------------===//
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

#include "language/AST/AvailabilityQuery.h"
#include "language/AST/ASTContext.h"
#include "language/AST/Decl.h"
#include "language/Basic/Platform.h"

using namespace language;

static void unpackVersion(const toolchain::VersionTuple &version,
                          toolchain::SmallVectorImpl<unsigned> &arguments) {
  arguments.push_back(version.getMajor());
  arguments.push_back(version.getMinor().value_or(0));
  arguments.push_back(version.getSubminor().value_or(0));
}

static FuncDecl *
getOSVersionRangeCheck(const toolchain::VersionTuple &version,
                       toolchain::SmallVectorImpl<unsigned> &arguments,
                       ASTContext &ctx, bool forTargetVariant) {
  unpackVersion(version, arguments);
  return forTargetVariant ? ctx.getIsVariantOSVersionAtLeastDecl()
                          : ctx.getIsOSVersionAtLeastDecl();
}

static FuncDecl *getOSVersionOrVariantVersionRangeCheck(
    const toolchain::VersionTuple &targetVersion,
    const toolchain::VersionTuple &variantVersion,
    toolchain::SmallVectorImpl<unsigned> &arguments, ASTContext &ctx) {
  unpackVersion(targetVersion, arguments);
  unpackVersion(variantVersion, arguments);
  return ctx.getIsOSVersionAtLeastOrVariantVersionAtLeast();
}

static FuncDecl *
getZipperedOSVersionRangeCheck(const AvailabilityQuery &query,
                               toolchain::SmallVectorImpl<unsigned> &arguments,
                               ASTContext &ctx) {

  auto targetVersion = query.getPrimaryArgument();
  auto variantVersion = query.getVariantArgument();
  DEBUG_ASSERT(targetVersion || variantVersion);

  // We're building zippered, so we need to pass both macOS and iOS versions to
  // the runtime version range check. At run time that check will determine what
  // kind of process this code is loaded into. In a macOS process it will use
  // the macOS version; in an macCatalyst process it will use the iOS version.
  toolchain::Triple targetTriple = ctx.LangOpts.Target;
  toolchain::Triple variantTriple = *ctx.LangOpts.TargetVariant;

  // From perspective of the driver and most of the frontend, -target and
  // -target-variant are symmetric. That is, the user can pass either:
  //    -target x86_64-apple-macosx10.15 \
  //    -target-variant x86_64-apple-ios13.1-macabi
  // or:
  //    -target x86_64-apple-ios13.1-macabi \
  //    -target-variant x86_64-apple-macosx10.15
  //
  // However, the runtime availability-checking entry points need to compare
  // against an actual running OS version and so can't be symmetric. Here we
  // standardize on "target" means macOS version and "targetVariant" means iOS
  // version.
  if (tripleIsMacCatalystEnvironment(targetTriple)) {
    DEBUG_ASSERT(variantTriple.isMacOSX());
    // Normalize so that "variant" always means iOS version.
    std::swap(targetVersion, variantVersion);
    std::swap(targetTriple, variantTriple);
  }

  // The variant-only availability-checking entrypoint is not part of the
  // Codira 5.0 ABI. It is only available in macOS 10.15 and above.
  bool isVariantEntrypointAvailable = !targetTriple.isMacOSXVersionLT(10, 15);

  // If there is no check for the target but there is for the variant, then we
  // only need to emit code for the variant check.
  if (isVariantEntrypointAvailable && !targetVersion && variantVersion)
    return getOSVersionRangeCheck(*variantVersion, arguments, ctx,
                                  /*forVariant=*/true);

  // Similarly, if there is a check for the target but not for the target
  // variant then we only to emit code for the target check.
  if (targetVersion && !variantVersion)
    return getOSVersionRangeCheck(*targetVersion, arguments, ctx,
                                  /*forTargetVariant=*/false);

  if (!isVariantEntrypointAvailable || (targetVersion && variantVersion)) {

    // If the variant-only entrypoint isn't available (as is the case
    // pre-macOS 10.15) we need to use the zippered entrypoint (which is part of
    // the Codira 5.0 ABI) even when the macOS version is '*' (all). In this
    // case, use the minimum macOS deployment version from the target triple.
    // This ensures the check always passes on macOS.
    if (!isVariantEntrypointAvailable && !targetVersion) {
      DEBUG_ASSERT(targetTriple.isMacOSX());

      toolchain::VersionTuple macosVersion;
      targetTriple.getMacOSXVersion(macosVersion);
      targetVersion = macosVersion;
    }

    return getOSVersionOrVariantVersionRangeCheck(
        *targetVersion, *variantVersion, arguments, ctx);
  }

  toolchain_unreachable("Unhandled zippered configuration");
}

static FuncDecl *
getOSAvailabilityDeclAndArguments(const AvailabilityQuery &query,
                                  toolchain::SmallVectorImpl<unsigned> &arguments,
                                  ASTContext &ctx) {
  if (ctx.LangOpts.TargetVariant)
    return getZipperedOSVersionRangeCheck(query, arguments, ctx);

  bool isMacCatalyst = tripleIsMacCatalystEnvironment(ctx.LangOpts.Target);
  return getOSVersionRangeCheck(query.getPrimaryArgument().value(), arguments,
                                ctx, isMacCatalyst);
}

FuncDecl *AvailabilityQuery::getDynamicQueryDeclAndArguments(
    toolchain::SmallVectorImpl<unsigned> &arguments, ASTContext &ctx) const {
  auto domain = getDomain();
  switch (domain.getKind()) {
  case AvailabilityDomain::Kind::Universal:
  case AvailabilityDomain::Kind::CodiraLanguage:
  case AvailabilityDomain::Kind::PackageDescription:
  case AvailabilityDomain::Kind::Embedded:
    return nullptr;
  case AvailabilityDomain::Kind::Platform:
    return getOSAvailabilityDeclAndArguments(*this, arguments, ctx);
  case AvailabilityDomain::Kind::Custom:
    auto customDomain = domain.getCustomDomain();
    ASSERT(customDomain);

    return customDomain->getPredicateFunc();
  }
}
