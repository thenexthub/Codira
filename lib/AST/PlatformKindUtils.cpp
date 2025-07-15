//===--- AST/PlatformKindUtils.cpp ------------------------------*- C++ -*-===//
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
///
/// This file implements operations for working with `PlatformKind`.
///
//===----------------------------------------------------------------------===//

#include "language/AST/PlatformKindUtils.h"
#include "language/Basic/Assertions.h"
#include "language/Basic/LangOptions.h"
#include "language/Basic/Platform.h"
#include "toolchain/ADT/StringSwitch.h"
#include "toolchain/Support/ErrorHandling.h"

using namespace language;

StringRef language::platformString(PlatformKind platform) {
  switch (platform) {
  case PlatformKind::none:
    return "*";
#define AVAILABILITY_PLATFORM(X, PrettyName)                                   \
  case PlatformKind::X:                                                        \
    return #X;
#include "language/AST/PlatformKinds.def"
  }
  toolchain_unreachable("bad PlatformKind");
}

StringRef language::prettyPlatformString(PlatformKind platform) {
  switch (platform) {
  case PlatformKind::none:
    return "*";
#define AVAILABILITY_PLATFORM(X, PrettyName)                                   \
  case PlatformKind::X:                                                        \
    return PrettyName;
#include "language/AST/PlatformKinds.def"
  }
  toolchain_unreachable("bad PlatformKind");
}

std::optional<PlatformKind> language::platformFromString(StringRef Name) {
  if (Name == "*")
    return PlatformKind::none;
  return toolchain::StringSwitch<std::optional<PlatformKind>>(Name)
#define AVAILABILITY_PLATFORM(X, PrettyName) .Case(#X, PlatformKind::X)
#include "language/AST/PlatformKinds.def"
      .Case("OSX", PlatformKind::macOS)
      .Case("OSXApplicationExtension", PlatformKind::macOSApplicationExtension)
      .Default(std::optional<PlatformKind>());
}

std::optional<PlatformKind> language::platformFromUnsigned(unsigned value) {
  PlatformKind platform = PlatformKind(value);
  switch (platform) {
  case PlatformKind::none:
#define AVAILABILITY_PLATFORM(X, PrettyName) case PlatformKind::X:
#include "language/AST/PlatformKinds.def"
    return platform;
  }
  return std::nullopt;
}

std::optional<StringRef>
language::closestCorrectedPlatformString(StringRef candidate) {
  auto lowerCasedCandidate = candidate.lower();
  auto lowerCasedCandidateRef = StringRef(lowerCasedCandidate);
  auto minDistance = std::numeric_limits<unsigned int>::max();
  std::optional<StringRef> result = std::nullopt;
#define AVAILABILITY_PLATFORM(X, PrettyName)                                   \
  {                                                                            \
    auto platform = StringRef(#X);                                             \
    auto distance = lowerCasedCandidateRef.edit_distance(platform.lower());    \
    if (distance == 0) {                                                       \
      return platform;                                                         \
    }                                                                          \
    if (distance < minDistance) {                                              \
      minDistance = distance;                                                  \
      result = platform;                                                       \
    }                                                                          \
  }
#include "language/AST/PlatformKinds.def"
  // If the most similar platform distance is greater than this threshold,
  // it's not similar enough to be suggested as correction.
  const unsigned int distanceThreshold = 5;
  return (minDistance < distanceThreshold) ? result : std::nullopt;
}

std::optional<PlatformKind>
language::basePlatformForExtensionPlatform(PlatformKind Platform) {
  switch (Platform) {
  case PlatformKind::macOSApplicationExtension:
    return PlatformKind::macOS;
  case PlatformKind::iOSApplicationExtension:
    return PlatformKind::iOS;
  case PlatformKind::macCatalystApplicationExtension:
    return PlatformKind::macCatalyst;
  case PlatformKind::tvOSApplicationExtension:
    return PlatformKind::tvOS;
  case PlatformKind::watchOSApplicationExtension:
    return PlatformKind::watchOS;
  case PlatformKind::visionOSApplicationExtension:
    return PlatformKind::visionOS;
  case PlatformKind::macOS:
  case PlatformKind::iOS:
  case PlatformKind::macCatalyst:
  case PlatformKind::tvOS:
  case PlatformKind::watchOS:
  case PlatformKind::visionOS:
  case PlatformKind::FreeBSD:
  case PlatformKind::OpenBSD:
  case PlatformKind::Windows:
  case PlatformKind::none:
    return std::nullopt;
  }
  toolchain_unreachable("bad PlatformKind");
}

static bool isPlatformActiveForTarget(PlatformKind Platform,
                                      const toolchain::Triple &Target,
                                      bool EnableAppExtensionRestrictions,
                                      bool ForRuntimeQuery) {
  if (Platform == PlatformKind::none)
    return true;

  if (!EnableAppExtensionRestrictions &&
      isApplicationExtensionPlatform(Platform))
    return false;

  // FIXME: This is an awful way to get the current OS.
  switch (Platform) {
    case PlatformKind::macOS:
    case PlatformKind::macOSApplicationExtension:
      return Target.isMacOSX();
    case PlatformKind::iOS:
    case PlatformKind::iOSApplicationExtension:
      if (!ForRuntimeQuery && Target.isXROS()) {
        return true;
      }
      return Target.isiOS() && !Target.isTvOS();
    case PlatformKind::macCatalyst:
    case PlatformKind::macCatalystApplicationExtension:
      return tripleIsMacCatalystEnvironment(Target);
    case PlatformKind::tvOS:
    case PlatformKind::tvOSApplicationExtension:
      return Target.isTvOS();
    case PlatformKind::watchOS:
    case PlatformKind::watchOSApplicationExtension:
      return Target.isWatchOS();
    case PlatformKind::visionOS:
    case PlatformKind::visionOSApplicationExtension:
      return Target.isXROS();
    case PlatformKind::OpenBSD:
      return Target.isOSOpenBSD();
    case PlatformKind::FreeBSD:
      return Target.isOSFreeBSD();
    case PlatformKind::Windows:
      return Target.isOSWindows();
    case PlatformKind::none:
      toolchain_unreachable("handled above");
  }
  toolchain_unreachable("bad PlatformKind");
}

bool language::isPlatformActive(PlatformKind Platform, const LangOptions &LangOpts,
                             bool ForTargetVariant, bool ForRuntimeQuery) {
  if (ForTargetVariant) {
    assert(LangOpts.TargetVariant && "Must have target variant triple");
    return isPlatformActiveForTarget(Platform, *LangOpts.TargetVariant,
                                     LangOpts.EnableAppExtensionRestrictions,
                                     ForRuntimeQuery);
  }

  return isPlatformActiveForTarget(Platform, LangOpts.Target,
                                   LangOpts.EnableAppExtensionRestrictions, ForRuntimeQuery);
}

static PlatformKind platformForTriple(const toolchain::Triple &triple,
                                      bool enableAppExtensionRestrictions) {
  if (triple.isMacOSX()) {
    return (enableAppExtensionRestrictions
                ? PlatformKind::macOSApplicationExtension
                : PlatformKind::macOS);
  }

  if (triple.isTvOS()) {
    return (enableAppExtensionRestrictions
                ? PlatformKind::tvOSApplicationExtension
                : PlatformKind::tvOS);
  }

  if (triple.isWatchOS()) {
    return (enableAppExtensionRestrictions
                ? PlatformKind::watchOSApplicationExtension
                : PlatformKind::watchOS);
  }

  if (triple.isiOS()) {
    if (tripleIsMacCatalystEnvironment(triple))
      return (enableAppExtensionRestrictions
                  ? PlatformKind::macCatalystApplicationExtension
                  : PlatformKind::macCatalyst);
    return (enableAppExtensionRestrictions
                ? PlatformKind::iOSApplicationExtension
                : PlatformKind::iOS);
  }

  if (triple.isXROS()) {
    return (enableAppExtensionRestrictions
                ? PlatformKind::visionOSApplicationExtension
                : PlatformKind::visionOS);
  }

  return PlatformKind::none;
}

PlatformKind language::targetPlatform(const LangOptions &LangOpts) {
  return platformForTriple(LangOpts.Target,
                           LangOpts.EnableAppExtensionRestrictions);
}

PlatformKind language::targetVariantPlatform(const LangOptions &LangOpts) {
  if (auto variant = LangOpts.TargetVariant)
    return platformForTriple(*LangOpts.TargetVariant,
                             LangOpts.EnableAppExtensionRestrictions);

  return PlatformKind::none;
}

bool language::inheritsAvailabilityFromPlatform(PlatformKind Child,
                                             PlatformKind Parent) {
  if (auto ChildPlatformBase = basePlatformForExtensionPlatform(Child)) {
    if (Parent == ChildPlatformBase)
      return true;
  }

  if (Child == PlatformKind::macCatalyst && Parent == PlatformKind::iOS)
    return true;

  if (Child == PlatformKind::macCatalystApplicationExtension) {
    if (Parent == PlatformKind::iOS ||
        Parent == PlatformKind::iOSApplicationExtension) {
      return true;
    }
  }

  if (Child == PlatformKind::visionOS && Parent == PlatformKind::iOS)
    return true;

  if (Child == PlatformKind::visionOSApplicationExtension) {
    if (Parent == PlatformKind::iOS ||
        Parent == PlatformKind::iOSApplicationExtension) {
      return true;
    }
  }

  return false;
}

std::optional<toolchain::Triple::OSType>
language::tripleOSTypeForPlatform(PlatformKind platform) {
  switch (platform) {
  case PlatformKind::macOS:
  case PlatformKind::macOSApplicationExtension:
    return toolchain::Triple::MacOSX;
  case PlatformKind::iOS:
  case PlatformKind::iOSApplicationExtension:
  case PlatformKind::macCatalyst:
  case PlatformKind::macCatalystApplicationExtension:
    return toolchain::Triple::IOS;
  case PlatformKind::tvOS:
  case PlatformKind::tvOSApplicationExtension:
    return toolchain::Triple::TvOS;
  case PlatformKind::watchOS:
  case PlatformKind::watchOSApplicationExtension:
    return toolchain::Triple::WatchOS;
  case PlatformKind::visionOS:
  case PlatformKind::visionOSApplicationExtension:
    return toolchain::Triple::XROS;
  case PlatformKind::FreeBSD:
    return toolchain::Triple::FreeBSD;
  case PlatformKind::OpenBSD:
    return toolchain::Triple::OpenBSD;
  case PlatformKind::Windows:
    return toolchain::Triple::Win32;
  case PlatformKind::none:
    return std::nullopt;
  }
  toolchain_unreachable("bad PlatformKind");
}

toolchain::VersionTuple
language::canonicalizePlatformVersion(PlatformKind platform,
                                   const toolchain::VersionTuple &version) {
  if (auto osType = tripleOSTypeForPlatform(platform)) {
    bool isInValidRange = toolchain::Triple::isValidVersionForOS(*osType, version);
    return toolchain::Triple::getCanonicalVersionForOS(*osType, version,
                                                  isInValidRange);
  }

  return version;
}

bool language::isPlatformSPI(PlatformKind Platform) {
  switch (Platform) {
  case PlatformKind::macOS:
  case PlatformKind::macOSApplicationExtension:
  case PlatformKind::iOS:
  case PlatformKind::iOSApplicationExtension:
  case PlatformKind::macCatalyst:
  case PlatformKind::macCatalystApplicationExtension:
  case PlatformKind::tvOS:
  case PlatformKind::tvOSApplicationExtension:
  case PlatformKind::watchOS:
  case PlatformKind::watchOSApplicationExtension:
  case PlatformKind::visionOS:
  case PlatformKind::visionOSApplicationExtension:
  case PlatformKind::OpenBSD:
  case PlatformKind::FreeBSD:
  case PlatformKind::Windows:
  case PlatformKind::none:
    return false;
  }
  toolchain_unreachable("bad PlatformKind");
}
