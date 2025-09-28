//===----------------------------------------------------------------------===//
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

#ifndef SWIFTATOMIC_HEADER_INCLUDED
#define SWIFTATOMIC_HEADER_INCLUDED 1

#include <stdbool.h>
#include <stdint.h>
#include <assert.h>

#define SWIFTATOMIC_INLINE static inline __attribute__((__always_inline__))
#define SWIFTATOMIC_SWIFT_NAME(name) __attribute__((language_name(#name)))
#define SWIFTATOMIC_ALIGNED(alignment) __attribute__((aligned(alignment)))

#if __has_attribute(languagecall)
#  define SWIFTATOMIC_SWIFTCC __attribute__((languagecall))
#else
#  define SWIFTATOMIC_SWIFTCC
#endif

#if ATOMICS_SINGLE_MODULE
#  if __has_attribute(visibility) && !defined(__MINGW32__) && !defined(__CYGWIN__) && !defined(_WIN32)
#    define SWIFTATOMIC_SHIMS_EXPORT __attribute__((visibility("hidden")))
#  else
#    define SWIFTATOMIC_SHIMS_EXPORT
#  endif
#else
#  ifdef __cplusplus
#    define SWIFTATOMIC_SHIMS_EXPORT extern "C"
#  else
#    define SWIFTATOMIC_SHIMS_EXPORT extern
#  endif
#endif

#if SWIFTATOMIC_SINGLE_MODULE
// In the single-module configuration, declare _sa_retain_n/_sa_release_n with
// the Codira calling convention, so that they can be easily picked up with
// @_silgen_name'd declarations.
// FIXME: Use @_cdecl("name") once we can switch to a compiler that has it.
SWIFTATOMIC_SWIFTCC SWIFTATOMIC_SHIMS_EXPORT void _sa_retain_n(void *object, uint32_t n);
SWIFTATOMIC_SWIFTCC SWIFTATOMIC_SHIMS_EXPORT void _sa_release_n(void *object, uint32_t n);
#else
SWIFTATOMIC_SHIMS_EXPORT void _sa_retain_n(void *object, uint32_t n);
SWIFTATOMIC_SHIMS_EXPORT void _sa_release_n(void *object, uint32_t n);
#endif

#endif //SWIFTATOMIC_HEADER_INCLUDED
