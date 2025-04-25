//===--- PlatformKind.cpp - Swift Language Platform Kinds -----------------===//
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
// This file implements the platform kinds for API availability.
//
//===----------------------------------------------------------------------===//

#include "language/AST/PlatformKind.h"
#include "language/Basic/Assertions.h"
#include "language/Basic/LangOptions.h"
#include "language/Basic/Platform.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/Support/ErrorHandling.h"


using namespace language;

StringRef swift::platformString(PlatformKind platform) {
  switch (platform) {
  case PlatformKind::none:
    return "*";
#define AVAILABILITY_PLATFORM(X, PrettyName)                                   \
  case PlatformKind::X:                                                        \
    return #X;
#include "language/AST/PlatformKinds.def"
  }
  llvm_unreachable("bad PlatformKind");
}

StringRef swift::prettyPlatformString(PlatformKind platform) {
  switch (platform) {
  case PlatformKind::none:
    return "*";
#define AVAILABILITY_PLATFORM(X, PrettyName)                                   \
  case PlatformKind::X:                                                        \
    return PrettyName;
#include "language/AST/PlatformKinds.def"
  }
  llvm_unreachable("bad PlatformKind");
}

std::optional<PlatformKind> swift::platformFromString(StringRef Name) {
  if (Name == "*")
    return PlatformKind::none;
  return llvm::StringSwitch<std::optional<PlatformKind>>(Name)
#define AVAILABILITY_PLATFORM(X, PrettyName) .Case(#X, PlatformKind::X)
#include "language/AST/PlatformKinds.def"
      .Case("OSX", PlatformKind::macOS)
      .Case("OSXApplicationExtension", PlatformKind::macOSApplicationExtension)
      .Default(std::optional<PlatformKind>());
}

std::optional<PlatformKind> swift::platformFromUnsigned(unsigned value) {
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
swift::closestCorrectedPlatformString(StringRef candidate) {
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
swift::basePlatformForExtensionPlatform(PlatformKind Platform) {
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
  case PlatformKind::OpenBSD:
  case PlatformKind::Windows:
  case PlatformKind::none:
    return std::nullopt;
  }
  llvm_unreachable("bad PlatformKind");
}

static bool isPlatformActiveForTarget(PlatformKind Platform,
                                      const llvm::Triple &Target,
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
    case PlatformKind::Windows:
      return Target.isOSWindows();
    case PlatformKind::none:
      llvm_unreachable("handled above");
  }
  llvm_unreachable("bad PlatformKind");
}

bool swift::isPlatformActive(PlatformKind Platform, const LangOptions &LangOpts,
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

static PlatformKind platformForTriple(const llvm::Triple &triple,
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

PlatformKind swift::targetPlatform(const LangOptions &LangOpts) {
  return platformForTriple(LangOpts.Target,
                           LangOpts.EnableAppExtensionRestrictions);
}

PlatformKind swift::targetVariantPlatform(const LangOptions &LangOpts) {
  if (auto variant = LangOpts.TargetVariant)
    return platformForTriple(*LangOpts.TargetVariant,
                             LangOpts.EnableAppExtensionRestrictions);

  return PlatformKind::none;
}

bool swift::inheritsAvailabilityFromPlatform(PlatformKind Child,
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

llvm::VersionTuple swift::canonicalizePlatformVersion(
    PlatformKind platform, const llvm::VersionTuple &version) {

  // Canonicalize macOS version for macOS Big Sur to treat
  // 10.16 as 11.0.
  if (platform == PlatformKind::macOS ||
      platform == PlatformKind::macOSApplicationExtension) {
    return llvm::Triple::getCanonicalVersionForOS(llvm::Triple::MacOSX,
                                                  version);
  }

  return version;
}

bool swift::isPlatformSPI(PlatformKind Platform) {
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
  case PlatformKind::Windows:
  case PlatformKind::none:
    return false;
  }
  llvm_unreachable("bad PlatformKind");
}
