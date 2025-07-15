//===--- Paths.h - Codira Runtime path utility functions ---------*- C++ -*-===//
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
// Functions that obtain paths that might be useful within the runtime.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_RUNTIME_UTILS_H
#define LANGUAGE_RUNTIME_UTILS_H

#include "language/Runtime/Config.h"

/// Return the path of the liblanguageCore library.
///
/// This can be used to locate files that are installed alongside the Codira
/// runtime library.
///
/// \return A string containing the full path to liblanguageCore.  The string is
///         owned by the runtime and should not be freed.
LANGUAGE_RUNTIME_EXPORT
const char *
language_getRuntimeLibraryPath();

/// Return the path of the Codira root.
///
/// If the path to liblanguageCore is `/usr/local/language/lib/liblanguageCore.dylib`,
/// this function would return `/usr/local/language`.
///
/// The path returned here can be overridden by setting the environment variable
/// LANGUAGE_ROOT.
///
/// \return A string containing the full path to the Codira root directory, based
///         either on the location of the Codira runtime, or on the `LANGUAGE_ROOT`
///         environment variable if set.  The string is owned by the runtime
///         and should not be freed.
LANGUAGE_RUNTIME_EXPORT
const char *
language_getRootPath();

/// Return the path of the specified auxiliary executable.
///
/// This function will search for the auxiliary executable in the following
/// paths:
///
///   <language-root>/libexec/language/<platform>/<name>
///   <language-root>/libexec/language/<name>
///   <language-root>/bin/<name>
///   <language-root>/<name>
///
/// It will return the first of those that exists, but it does not test that
/// the file is indeed executable.
///
/// On Windows, it will automatically add `.exe` to the name, which means you
/// do not need to special case the name for Windows.
///
/// If you are using this function to locate a utility program for use by the
/// runtime, you should provide a way to override its location using an
/// environment variable.
///
/// If the executable cannot be found, it will return nullptr.
///
/// \param name      The name of the executable to locate.
///
/// \return A string containing the full path to the executable.  This string
///         should be released with `free()` when no longer required.
LANGUAGE_RUNTIME_EXPORT
char *
language_copyAuxiliaryExecutablePath(const char *name);

#endif // LANGUAGE_RUNTIME_PATHS_H
