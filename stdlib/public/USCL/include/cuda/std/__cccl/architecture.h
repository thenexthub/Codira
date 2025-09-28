/*
 *
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 * Date: Sunday, April 14, 2024.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 *
 */

#ifndef __CCCL_ARCH_H
#define __CCCL_ARCH_H

#include <uscl/std/__cccl/compiler.h>
#include <uscl/std/__cccl/preprocessor.h>

// The header provides the following macros to determine the host architecture:
//
// _CCCL_ARCH(ARM64)     ARM64
// _CCCL_ARCH(X86_64)    X86 64 bit

// Determine the host compiler and its version

// Arm 64-bit
#if (defined(__aarch64__) || defined(_M_ARM64) || defined(_M_ARM64EC) /*emulation*/)
#  define _CCCL_ARCH_ARM64_() 1
#else
#  define _CCCL_ARCH_ARM64_() 0
#endif

// X86 64-bit

// _M_X64 is defined even if we are compiling in Arm64 emulation mode
#if (defined(_M_X64) && !defined(_M_ARM64EC)) || defined(__amd64__) || defined(__x86_64__)
#  define _CCCL_ARCH_X86_64_() 1
#else
#  define _CCCL_ARCH_X86_64_() 0
#endif

#define _CCCL_ARCH(...) _CCCL_ARCH_##__VA_ARGS__##_()

// Determine the endianness

#define _CCCL_ENDIAN_LITTLE() 0xDEAD
#define _CCCL_ENDIAN_BIG()    0xFACE
#define _CCCL_ENDIAN_PDP()    0xBEEF

#if _CCCL_COMPILER(NVRTC) || (_CCCL_COMPILER(MSVC) && (_CCCL_ARCH(X86_64) || _CCCL_ARCH(ARM64))) || __LITTLE_ENDIAN__
#  define _CCCL_ENDIAN_NATIVE() _CCCL_ENDIAN_LITTLE()
#elif __BIG_ENDIAN__
#  define _CCCL_ENDIAN_NATIVE() _CCCL_ENDIAN_BIG()
#elif defined(__BYTE_ORDER__)
#  if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#    define _CCCL_ENDIAN_NATIVE() _CCCL_ENDIAN_LITTLE()
#  elif __BYTE_ORDER__ == __ORDER_PDP_ENDIAN__
#    define _CCCL_ENDIAN_NATIVE() _CCCL_ENDIAN_PDP()
#  elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#    define _CCCL_ENDIAN_NATIVE() _CCCL_ENDIAN_BIG()
#  endif // __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#elif _CCCL_HAS_INCLUDE(<endian.h>)
#  include <endian.h>
#  if __BYTE_ORDER == __LITTLE_ENDIAN
#    define _CCCL_ENDIAN_NATIVE() _CCCL_ENDIAN_LITTLE()
#  elif __BYTE_ORDER == __PDP_ENDIAN
#    define _CCCL_ENDIAN_NATIVE() _CCCL_ENDIAN_PDP()
#  elif __BYTE_ORDER == __BIG_ENDIAN
#    define _CCCL_ENDIAN_NATIVE() _CCCL_ENDIAN_BIG()
#  endif // __BYTE_ORDER == __BIG_ENDIAN
#endif // ^^^ has endian.h ^^^

#if !defined(_CCCL_ENDIAN_NATIVE)
_CCCL_WARNING("failed to determine the endianness of the host architecture, defaulting to little-endian")
#  define _CCCL_ENDIAN_NATIVE() _CCCL_ENDIAN_LITTLE()
#endif // !_CCCL_ENDIAN_NATIVE

#define _CCCL_ENDIAN(_NAME) (_CCCL_ENDIAN_NATIVE() == _CCCL_ENDIAN_##_NAME())

#endif // __CCCL_ARCH_H
