//===-- Wrapper for C standard stdlib.h declarations on the GPU -----------===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
// 
// Author: Tunjay Akbarli
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// 
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//

#ifndef __CLANG_LLVM_LIBC_WRAPPERS_STDLIB_H__
#define __CLANG_LLVM_LIBC_WRAPPERS_STDLIB_H__

#if !defined(_OPENMP) && !defined(__HIP__) && !defined(__CUDA__)
#error "This file is for GPU offloading compilation only"
#endif

#include_next <stdlib.h>

#if __has_include(<toolchain-libc-decls/stdlib.h>)

#if defined(__HIP__) || defined(__CUDA__)
#define __LIBC_ATTRS __attribute__((device))
#endif

#pragma omp begin declare target

// The LLVM C library uses these named types so we forward declare them.
typedef void (*__atexithandler_t)(void);
typedef int (*__search_compare_t)(const void *, const void *);
typedef int (*__qsortcompare_t)(const void *, const void *);
typedef int (*__qsortrcompare_t)(const void *, const void *, void *);

// Enforce ABI compatibility with the structs used by the LLVM C library.
_Static_assert(__builtin_offsetof(div_t, quot) == 0, "ABI mismatch!");
_Static_assert(__builtin_offsetof(ldiv_t, quot) == 0, "ABI mismatch!");
_Static_assert(__builtin_offsetof(lldiv_t, quot) == 0, "ABI mismatch!");

#if defined(__GLIBC__) && __cplusplus >= 201703L
#define at_quick_exit atexit
#endif

#include <toolchain-libc-decls/stdlib.h>

#if defined(__GLIBC__) && __cplusplus >= 201703L
#undef at_quick_exit
#endif

#pragma omp end declare target

#undef __LIBC_ATTRS

#endif

#endif // __CLANG_LLVM_LIBC_WRAPPERS_STDLIB_H__
