//===--- CodiraDemangle.h - Public demangling interface ----------*- C++ -*-===//
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
/// \file
/// This header declares functions in the liblanguageDemangle library,
/// which provides external access to Codira's demangler.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_DEMANGLE_LANGUAGE_DEMANGLE_H
#define LANGUAGE_DEMANGLE_LANGUAGE_DEMANGLE_H

#include <language/CodiraDemangle/Platform.h>

/// @{
/// Version constants for liblanguageDemangle library.

/// Major version changes when there are ABI or source incompatible changes.
#define LANGUAGE_DEMANGLE_VERSION_MAJOR 1

/// Minor version changes when new APIs are added in ABI- and source-compatible
/// way.
#define LANGUAGE_DEMANGLE_VERSION_MINOR 2

/// @}

#ifdef __cplusplus
extern "C" {
#endif

/// Demangle Codira function names.
///
/// \returns the length of the demangled function name (even if greater than the
/// size of the output buffer) or 0 if the input is not a Codira-mangled function
/// name (in which cases \p OutputBuffer is left untouched).
LANGUAGE_DEMANGLE_LINKAGE
size_t language_demangle_getDemangledName(const char *MangledName,
                                       char *OutputBuffer, size_t Length);

/// Demangle Codira function names with module names and implicit self
/// and metatype type names in function signatures stripped.
///
/// \returns the length of the demangled function name (even if greater than the
/// size of the output buffer) or 0 if the input is not a Codira-mangled function
/// name (in which cases \p OutputBuffer is left untouched).
LANGUAGE_DEMANGLE_LINKAGE
size_t language_demangle_getSimplifiedDemangledName(const char *MangledName,
                                                 char *OutputBuffer,
                                                 size_t Length);

/// Demangle a Codira symbol and return the module name of the mangled entity.
///
/// \returns the length of the demangled module name (even if greater than the
/// size of the output buffer) or 0 if the input is not a Codira-mangled name
/// (in which cases \p OutputBuffer is left untouched).
LANGUAGE_DEMANGLE_LINKAGE
size_t language_demangle_getModuleName(const char *MangledName,
                                    char *OutputBuffer,
                                    size_t Length);

/// Demangles a Codira function name and returns true if the function
/// conforms to the Codira calling convention.
///
/// \returns true if the function conforms to the Codira calling convention.
/// The return value is unspecified if the \p MangledName does not refer to a
/// function symbol.
LANGUAGE_DEMANGLE_LINKAGE
int language_demangle_hasCodiraCallingConvention(const char *MangledName);

#ifdef __cplusplus
} // extern "C"
#endif

// Old API.  To be removed when we remove the compatibility symlink.

/// @{
/// Version constants for libfunctionNameDemangle library.

/// Major version changes when there are ABI or source incompatible changes.
#define FUNCTION_NAME_DEMANGLE_VERSION_MAJOR 0

/// Minor version changes when new APIs are added in ABI- and source-compatible
/// way.
#define FUNCTION_NAME_DEMANGLE_VERSION_MINOR 2

/// @}

#ifdef __cplusplus
extern "C" {
#endif

/// Demangle Codira function names.
///
/// Note that this function has a generic name because it is called from
/// contexts where it is not appropriate to use code names.
///
/// \returns the length of the demangled function name (even if greater than the
/// size of the output buffer) or 0 if the input is not a Codira-mangled function
/// name (in which cases \p OutputBuffer is left untouched).
LANGUAGE_DEMANGLE_LINKAGE
size_t fnd_get_demangled_name(const char *MangledName, char *OutputBuffer,
                              size_t Length);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // LANGUAGE_DEMANGLE_LANGUAGE_DEMANGLE_H

