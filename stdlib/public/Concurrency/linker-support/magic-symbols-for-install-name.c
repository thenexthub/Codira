//===--- magic-symbols-for-install-name.c - Magic linker directive symbols ===//
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
// A file containing magic symbols that instruct the linker to use a
// different install name when targeting older OSes. This file gets
// compiled into all of the libraries that are embedded for backward
// deployment.
//
// This file is specific to the Concurrency library; there is a matching file
// for the standard library with the same name.
//
//===----------------------------------------------------------------------===//

#if defined(__APPLE__) && defined(__MACH__) && LANGUAGE_STDLIB_SUPPORT_BACK_DEPLOYMENT

#include <mach-o/loader.h>
#include <TargetConditionals.h>
#include "language/shims/Visibility.h"

// Concurrency was supported as an embedded library in macOS 10.15, iOS 13.0, watchOS 6.0, tvOS 13.0. It
// became part of the OS in macOS 12.0, iOS 15.0, watchOS 8.0, tvOS 15.0. Projects can continue to embed
// Concurrency, but the linker will see the OS version and try to link on that by default. In order to
// support back deployment, add a magic symbol to the OS library so that back deployment will link on the
// embedded library instead. When running on a newer OS, the OS version of the library will be used due to
// Xcode inserting a runpath search path of /usr/lib/language based on the deployment target being less than
// SupportedTargets[target][CodiraConcurrencyMinimumDeploymentTarget] in SDKSettings.plist.

// Clients can back deploy to OS versions that predate Concurrency as an embedded library, and conditionally
// use it behind an #availability check. Such clients will still need to link the embedded library instead
// of the OS version. To support that, set the start version to Codira's first supported versions: macOS (née
// OS X) 10.9, iOS 7.0, watchOS 2.0, tvOS 9.0 rather than Concurrency's first supported versions listed
// above.

// The linker uses a specially formatted symbol to do the back deployment:
// $ld$previous$<install-name>$<compatibility-version>$<platform>$<start-version>$<end-version>$<symbol-name>$
// compatibility-version and symbol-name are left off to apply to all library versions and symbols.
// This symbol isn't a legal C identifier, so it needs to be specified with __asm.
#define RPATH_PREVIOUS_DIRECTIVE_IMPL(name, platform, startVersion, endVersion) \
  LANGUAGE_RUNTIME_EXPORT const char ld_previous_ ## platform \
  __asm("$ld$previous$@rpath/lib" __STRING(name) ".dylib$$" __STRING(platform) "$" __STRING(startVersion) "$" __STRING(endVersion) "$$"); \
  const char ld_previous_ ## platform = 0;
// Using the __STRING macro is important so that name and platform get expanded before being
// stringified. The versions could just be #version, __STRING is only used for consistency.

#define RPATH_PREVIOUS_DIRECTIVE(platform, startVersion, endVersion) \
  RPATH_PREVIOUS_DIRECTIVE_IMPL(LANGUAGE_TARGET_LIBRARY_NAME, platform, startVersion, endVersion)

#if TARGET_OS_OSX || TARGET_OS_MACCATALYST
RPATH_PREVIOUS_DIRECTIVE(PLATFORM_MACOS, 10.9, 12.0)
RPATH_PREVIOUS_DIRECTIVE(PLATFORM_MACCATALYST, 13.1, 15.0)
#elif TARGET_OS_IOS && !TARGET_OS_VISION
#if TARGET_OS_SIMULATOR
RPATH_PREVIOUS_DIRECTIVE(PLATFORM_IOSSIMULATOR, 7.0, 15.0)
#else
RPATH_PREVIOUS_DIRECTIVE(PLATFORM_IOS, 7.0, 15.0)
#endif
#elif TARGET_OS_WATCH
#if TARGET_OS_SIMULATOR
RPATH_PREVIOUS_DIRECTIVE(PLATFORM_WATCHOSSIMULATOR, 2.0, 8.0)
#else
RPATH_PREVIOUS_DIRECTIVE(PLATFORM_WATCHOS, 2.0, 8.0)
#endif
#elif TARGET_OS_TV
#if TARGET_OS_SIMULATOR
RPATH_PREVIOUS_DIRECTIVE(PLATFORM_TVOSSIMULATOR, 9.0, 15.0)
#else
RPATH_PREVIOUS_DIRECTIVE(PLATFORM_TVOS, 9.0, 15.0)
#endif
#endif
// Concurrency wasn't supported as an embedded library in any other OS, so no need to create back deployment
// symbols for any of the other ones.

#endif // defined(__APPLE__) && defined(__MACH__) && LANGUAGE_STDLIB_SUPPORT_BACK_DEPLOYMENT
