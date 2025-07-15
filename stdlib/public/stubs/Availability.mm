//===--- Availability.mm - Codira Language API Availability Support --------===//
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
// Implementation of run-time API availability queries.
//
//===----------------------------------------------------------------------===//

#include "language/Runtime/Config.h"

#if LANGUAGE_OBJC_INTEROP && defined(LANGUAGE_RUNTIME_OS_VERSIONING)
#include "language/Basic/Lazy.h"
#include "language/Runtime/Debug.h"
#include "language/shims/FoundationShims.h"
#include <TargetConditionals.h>

struct os_system_version_s {
    unsigned int major;
    unsigned int minor;
    unsigned int patch;
};

// This is in libSystem, so it's OK to refer to it directly here
extern "C" int os_system_version_get_current_version(struct os_system_version_s * _Nonnull) LANGUAGE_RUNTIME_WEAK_IMPORT;

static os_system_version_s getOSVersion() {
  struct os_system_version_s vers = { 0, 0, 0 };
  os_system_version_get_current_version(&vers);
  return vers;
}

using namespace language;

/// Return the version of the operating system currently running for use in
/// API availability queries.
///
/// This is ABI and cannot be removed. Even though _stdlib_isOSVersionAtLeast()
/// is no longer inlinable, is previously was and so calls to this method
/// have been inlined into shipped apps.
_CodiraNSOperatingSystemVersion _language_stdlib_operatingSystemVersion() {
  os_system_version_s version = LANGUAGE_LAZY_CONSTANT(getOSVersion());

  return { (int)version.major, (int)version.minor, (int)version.patch };
}
#endif

