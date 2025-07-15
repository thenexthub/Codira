//===--- Platform.h - Helpers related to target platforms -------*- C++ -*-===//
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

#ifndef LANGUAGE_BASIC_PLATFORM_H
#define LANGUAGE_BASIC_PLATFORM_H

#include "language/Basic/Toolchain.h"
#include "language/Config.h"
#include "clang/Basic/DarwinSDKInfo.h"
#include "toolchain/ADT/StringRef.h"
#include <optional>

namespace toolchain {
  class Triple;
  class VersionTuple;
}

namespace language {

  enum class DarwinPlatformKind : unsigned {
    MacOS,
    IPhoneOS,
    IPhoneOSSimulator,
    TvOS,
    TvOSSimulator,
    WatchOS,
    WatchOSSimulator,
    VisionOS,
    VisionOSSimulator
  };

  /// Returns true if the given triple represents iOS running in a simulator.
  bool tripleIsiOSSimulator(const toolchain::Triple &triple);

  /// Returns true if the given triple represents AppleTV running in a simulator.
  bool tripleIsAppleTVSimulator(const toolchain::Triple &triple);

  /// Returns true if the given triple represents watchOS running in a simulator.
  bool tripleIsWatchSimulator(const toolchain::Triple &triple);

  /// Returns true if the given triple represents a macCatalyst environment.
  bool tripleIsMacCatalystEnvironment(const toolchain::Triple &triple);

  /// Returns true if the given triple represents visionOS running in a simulator.
  bool tripleIsVisionSimulator(const toolchain::Triple &triple);

  /// Determine whether the triple infers the "simulator" environment.
  bool tripleInfersSimulatorEnvironment(const toolchain::Triple &triple);

  /// Returns true if the given -target triple and -target-variant triple
  /// can be zippered.
  bool triplesAreValidForZippering(const toolchain::Triple &target,
                                   const toolchain::Triple &targetVariant);

  /// Returns the VersionTuple at which Codira first became available for the OS
  /// represented by `triple`.
  const std::optional<toolchain::VersionTuple>
  minimumAvailableOSVersionForTriple(const toolchain::Triple &triple);

  /// Returns true if the given triple represents an OS that has all the
  /// "built-in" ABI-stable libraries (stdlib and _Concurrency)
  /// (eg. in /usr/lib/language).
  bool tripleRequiresRPathForCodiraLibrariesInOS(const toolchain::Triple &triple);

  /// Returns true if the given triple represents a version of OpenBSD
  /// that enforces BTCFI by default.
  bool tripleBTCFIByDefaultInOpenBSD(const toolchain::Triple &triple);

  /// Returns the platform name for a given target triple.
  ///
  /// For example, the iOS simulator has the name "iphonesimulator", while real
  /// iOS uses "iphoneos". OS X is "macosx". (These names are intended to be
  /// compatible with Xcode's SDKs.)
  ///
  /// If the triple does not correspond to a known platform, the empty string is
  /// returned.
  StringRef getPlatformNameForTriple(const toolchain::Triple &triple);

  /// Returns the platform Kind for Darwin triples.
  DarwinPlatformKind getDarwinPlatformKind(const toolchain::Triple &triple);

  /// Returns the architecture component of the path for a given target triple.
  ///
  /// Typically this is used for mapping the architecture component of the
  /// path.
  ///
  /// For example, on Linux "armv6l" and "armv7l" are mapped to "armv6" and
  /// "armv7", respectively, within LLVM. Therefore the component path for the
  /// architecture specific objects will be found in their "mapped" paths.
  ///
  /// This is a stop-gap until full Triple support (ala Clang) exists within languagec.
  StringRef getMajorArchitectureName(const toolchain::Triple &triple);

  /// Computes the normalized target triple used as the most preferred name for
  /// module loading.
  ///
  /// For platforms with fat binaries, this canonicalizes architecture,
  /// vendor, and OS names, strips OS versions, and makes inferred environments
  /// explicit. For other platforms, it returns the unmodified triple.
  ///
  /// The input triple should already be "normalized" in the sense that
  /// toolchain::Triple::normalize() would not affect it.
  toolchain::Triple getTargetSpecificModuleTriple(const toolchain::Triple &triple);
  
  /// Computes the target triple without version information.
  toolchain::Triple getUnversionedTriple(const toolchain::Triple &triple);

  /// Get the Codira runtime version to deploy back to, given a deployment target expressed as an
  /// LLVM target triple.
  std::optional<toolchain::VersionTuple>
  getCodiraRuntimeCompatibilityVersionForTarget(const toolchain::Triple &Triple);

  /// Retrieve the target SDK version for the given SDKInfo and target triple.
  toolchain::VersionTuple getTargetSDKVersion(clang::DarwinSDKInfo &SDKInfo,
                                         const toolchain::Triple &triple);

  /// Compute a target triple that is canonicalized using the passed triple.
  /// \returns nullopt if computation fails.
  std::optional<toolchain::Triple> getCanonicalTriple(const toolchain::Triple &triple);

  /// Compare triples for equality but also including OSVersion.
  inline bool areTriplesStrictlyEqual(const toolchain::Triple &lhs,
                                      const toolchain::Triple &rhs) {
    return (lhs == rhs) && (lhs.getOSVersion() == rhs.getOSVersion());
  }

  /// Get SDK build version.
  std::string getSDKBuildVersion(StringRef SDKPath);
  std::string getSDKBuildVersionFromPlist(StringRef Path);

  /// Get SDK name.
  std::string getSDKName(StringRef SDKPath);
} // end namespace language

#endif // LANGUAGE_BASIC_PLATFORM_H

