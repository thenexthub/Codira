//===-- Wrapper for C standard stdio.h declarations on the GPU ------------===//
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

#if !defined(_OPENMP) && !defined(__HIP__) && !defined(__CUDA__)
#error "This file is for GPU offloading compilation only"
#endif

#include_next <stdio.h>

// In some old versions of glibc, other standard headers sometimes define
// special macros (e.g., __need_FILE) before including stdio.h to cause stdio.h
// to produce special definitions.  Future includes of stdio.h when those
// special macros are undefined are expected to produce the normal definitions
// from stdio.h.
//
// We do not apply our include guard (__CLANG_LLVM_LIBC_WRAPPERS_STDIO_H__)
// unconditionally to the above include_next.  Otherwise, after an occurrence of
// the first glibc stdio.h use case described above, the include_next would be
// skipped for remaining includes of stdio.h, leaving required symbols
// undefined.
//
// We make the following assumptions to handle all use cases:
//
// 1. If the above include_next produces special glibc definitions, then (a) it
//    does not produce the normal definitions that we must intercept below, (b)
//    the current file was included from a glibc header that already defined
//    __GLIBC__ (usually by including glibc's <features.h>), and (c) the above
//    include_next does not define _STDIO_H.  In that case, we skip the rest of
//    the current file and don't guard against future includes.
// 2. If the above include_next produces the normal stdio.h definitions, then
//    either (a) __GLIBC__ is not defined because C headers are from some other
//    libc implementation or (b) the above include_next defines _STDIO_H to
//    prevent the above include_next from having any effect in the future.
#if !defined(__GLIBC__) || defined(_STDIO_H)

#ifndef __CLANG_LLVM_LIBC_WRAPPERS_STDIO_H__
#define __CLANG_LLVM_LIBC_WRAPPERS_STDIO_H__

#if __has_include(<toolchain-libc-decls/stdio.h>)

#if defined(__HIP__) || defined(__CUDA__)
#define __LIBC_ATTRS __attribute__((device))
#endif

// Some headers provide these as macros. Temporarily undefine them so they do
// not conflict with any definitions for the GPU.

#pragma push_macro("stdout")
#pragma push_macro("stdin")
#pragma push_macro("stderr")

#undef stdout
#undef stderr
#undef stdin

#pragma omp begin declare target

#include <toolchain-libc-decls/stdio.h>

#pragma omp end declare target

#undef __LIBC_ATTRS

// Restore the original macros when compiling on the host.
#if !defined(__NVPTX__) && !defined(__AMDGPU__)
#pragma pop_macro("stdout")
#pragma pop_macro("stderr")
#pragma pop_macro("stdin")
#endif

#endif

#endif // __CLANG_LLVM_LIBC_WRAPPERS_STDIO_H__

#endif
