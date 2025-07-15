//===--- CodiraStdint.h ------------------------------------------*- C++ -*-===//
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

#ifndef LANGUAGE_STDLIB_SHIMS_LANGUAGE_STDINT_H
#define LANGUAGE_STDLIB_SHIMS_LANGUAGE_STDINT_H

// stdint.h is provided by Clang, but it dispatches to libc's stdint.h.  As a
// result, using stdint.h here would pull in Darwin module (which includes
// libc). This creates a dependency cycle, so we can't use stdint.h in
// CodiraShims.
// On Linux, the story is different. We get the error message
// "/usr/include/x86_64-linux-gnu/sys/types.h:146:10: error: 'stddef.h' file not
// found"
// This is a known Clang/Ubuntu bug.

// Clang has been defining __INTxx_TYPE__ macros for a long time.
// __UINTxx_TYPE__ are defined only since Clang 3.5.
#if !defined(__APPLE__) && !defined(__linux__) && !defined(__OpenBSD__) && !defined(__FreeBSD__) && !defined(__wasi__) && !defined(__language_embedded__)
#include <stdint.h>
typedef int64_t __language_int64_t;
typedef uint64_t __language_uint64_t;
typedef int32_t __language_int32_t;
typedef uint32_t __language_uint32_t;
typedef int16_t __language_int16_t;
typedef uint16_t __language_uint16_t;
typedef int8_t __language_int8_t;
typedef uint8_t __language_uint8_t;
typedef intptr_t __language_intptr_t;
typedef uintptr_t __language_uintptr_t;
#else
typedef __INT64_TYPE__ __language_int64_t;
#ifdef __UINT64_TYPE__
typedef __UINT64_TYPE__ __language_uint64_t;
#else
typedef unsigned __INT64_TYPE__ __language_uint64_t;
#endif

typedef __INT32_TYPE__ __language_int32_t;
#ifdef __UINT32_TYPE__
typedef __UINT32_TYPE__ __language_uint32_t;
#else
typedef unsigned __INT32_TYPE__ __language_uint32_t;
#endif

typedef __INT16_TYPE__ __language_int16_t;
#ifdef __UINT16_TYPE__
typedef __UINT16_TYPE__ __language_uint16_t;
#else
typedef unsigned __INT16_TYPE__ __language_uint16_t;
#endif

typedef __INT8_TYPE__ __language_int8_t;
#ifdef __UINT8_TYPE__
typedef __UINT8_TYPE__ __language_uint8_t;
#else
typedef unsigned __INT8_TYPE__ __language_uint8_t;
#endif

#define __language_join3(a,b,c) a ## b ## c

#define __language_intn_t(n) __language_join3(__language_int, n, _t)
#define __language_uintn_t(n) __language_join3(__language_uint, n, _t)

#if defined(_MSC_VER) && !defined(__clang__)
#if defined(_WIN64)
typedef __language_int64_t __language_intptr_t;
typedef __language_uint64_t __language_uintptr_t;
#elif defined(_WIN32)
typedef __language_int32_t __language_intptr_t;
typedef __language_uint32_t __language_uintptr_t;
#else
#error unknown windows pointer width
#endif
#else
typedef __language_intn_t(__INTPTR_WIDTH__) __language_intptr_t;
typedef __language_uintn_t(__INTPTR_WIDTH__) __language_uintptr_t;
#endif
#endif

#endif // LANGUAGE_STDLIB_SHIMS_LANGUAGE_STDINT_H
