//===--- AST/PlatformKindUtils.h --------------------------------*- C++ -*-===//
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
/// This file declares operations for working with `PlatformKind`.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_AST_PLATFORM_KIND_UTILS_H
#define LANGUAGE_AST_PLATFORM_KIND_UTILS_H

#include "language/AST/PlatformKind.h"
#include "language/Basic/Toolchain.h"
#include "language/Config.h"
#include "toolchain/Support/VersionTuple.h"
#include "toolchain/TargetParser/Triple.h"
#include <optional>

namespace language {

class LangOptions;

/// Returns the short string representing the platform, suitable for
/// use in availability specifications (e.g., "OSX").
StringRef platformString(PlatformKind platform);

/// Returns the platform kind corresponding to the passed-in short platform name
/// or None if such a platform kind does not exist.
std::optional<PlatformKind> platformFromString(StringRef Name);

/// Safely converts the given unsigned value to a valid \c PlatformKind value or
/// \c nullopt otherwise.
std::optional<PlatformKind> platformFromUnsigned(unsigned value);

/// Returns a valid platform string that is closest to the candidate string
/// based on edit distance. Returns \c None if the closest valid platform
/// distance is not within a minimum threshold.
std::optional<StringRef> closestCorrectedPlatformString(StringRef candidate);

/// Returns a human-readable version of the platform name as a string, suitable
/// for emission in diagnostics (e.g., "macOS").
StringRef prettyPlatformString(PlatformKind platform);

/// Returns the base platform for an application-extension platform. For example
/// `iOS` would be returned for `iOSApplicationExtension`. Returns `None` for
/// platforms which are not application extension platforms.
std::optional<PlatformKind>
basePlatformForExtensionPlatform(PlatformKind Platform);

/// Returns true if \p Platform represents and application extension platform,
/// e.g. `iOSApplicationExtension`.
inline bool isApplicationExtensionPlatform(PlatformKind Platform) {
  return basePlatformForExtensionPlatform(Platform).has_value();
}

/// Returns whether the passed-in platform is active, given the language
/// options. A platform is active if either it is the target platform or its
/// AppExtension variant is the target platform. For example, OS X is
/// considered active when the target operating system is OS X and app extension
/// restrictions are enabled, but OSXApplicationExtension is not considered
/// active when the target platform is OS X and app extension restrictions are
/// disabled. PlatformKind::none is always considered active.
/// If ForTargetVariant is true then for zippered builds the target-variant
/// triple will be used rather than the target to determine whether the
/// platform is active.
bool isPlatformActive(PlatformKind Platform, const LangOptions &LangOpts,
                      bool ForTargetVariant = false, bool ForRuntimeQuery = false);

/// Returns the target platform for the given language options.
PlatformKind targetPlatform(const LangOptions &LangOpts);

/// Returns the target variant platform for the given language options.
PlatformKind targetVariantPlatform(const LangOptions &LangOpts);

/// Returns true when availability attributes from the "parent" platform
/// should also apply to the "child" platform for declarations without
/// an explicit attribute for the child.
bool inheritsAvailabilityFromPlatform(PlatformKind Child, PlatformKind Parent);

/// Returns the LLVM triple OS type for the given platform, if there is one.
std::optional<toolchain::Triple::OSType>
tripleOSTypeForPlatform(PlatformKind platform);

toolchain::VersionTuple canonicalizePlatformVersion(
    PlatformKind platform, const toolchain::VersionTuple &version);

/// Returns true if \p Platform should be considered to be SPI and therefore not
/// printed in public `.codeinterface` files, for example.
bool isPlatformSPI(PlatformKind Platform);

} // end namespace language

#endif // LANGUAGE_AST_PLATFORM_KIND_UTILS_H
