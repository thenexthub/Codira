//===-- CodiraDemangle/Platform.h - CodiraDemangle Platform Decls -*- C++ -*-===//
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

#ifndef LANGUAGE_DEMANGLE_PLATFORM_H
#define LANGUAGE_DEMANGLE_PLATFORM_H

#if defined(__cplusplus)
extern "C" {
#endif

#if defined(languageDemangle_EXPORTS)
# if defined(__ELF__)
#   define LANGUAGE_DEMANGLE_LINKAGE __attribute__((__visibility__("protected")))
# elif defined(__MACH__)
#   define LANGUAGE_DEMANGLE_LINKAGE __attribute__((__visibility__("default")))
# else
#   define LANGUAGE_DEMANGLE_LINKAGE __declspec(dllexport)
# endif
#else
# if defined(__ELF__)
#   define LANGUAGE_DEMANGLE_LINKAGE __attribute__((__visibility__("default")))
# elif defined(__MACH__)
#   define LANGUAGE_DEMANGLE_LINKAGE __attribute__((__visibility__("default")))
# else
#   define LANGUAGE_DEMANGLE_LINKAGE __declspec(dllimport)
# endif
#endif

#if defined(__cplusplus)
}
#endif

#endif


