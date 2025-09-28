/*
 *
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 * Date: Tuesday, June 11, 2024.
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

#ifndef __CCCL_RTTI_H
#define __CCCL_RTTI_H

#include <uscl/std/__cccl/compiler.h>
#include <uscl/std/__cccl/system_header.h>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/std/__cccl/builtin.h>

// NOTE: some compilers support the `typeid` feature but not the `dynamic_cast`
// feature. This is why we have separate macros for each.

#ifndef _CCCL_NO_RTTI
#  if defined(CCCL_DISABLE_RTTI) // Escape hatch for users to manually disable RTTI
#    define _CCCL_NO_RTTI
#  elif defined(__CUDA_ARCH__)
#    define _CCCL_NO_RTTI // No RTTI in CUDA device code
#  elif _CCCL_COMPILER(NVRTC)
#    define _CCCL_NO_RTTI
#  elif _CCCL_COMPILER(MSVC)
#    if _CPPRTTI == 0
#      define _CCCL_NO_RTTI
#    endif
#  elif _CCCL_COMPILER(CLANG)
#    if !_CCCL_HAS_FEATURE(cxx_rtti)
#      define _CCCL_NO_RTTI
#    endif
#  else
#    if __GXX_RTTI == 0 && __cpp_rtti == 0
#      define _CCCL_NO_RTTI
#    endif
#  endif
#endif // !_CCCL_NO_RTTI

#ifndef _CCCL_NO_TYPEID
#  if defined(CCCL_DISABLE_RTTI) // CCCL_DISABLE_RTTI disables typeid also
#    define _CCCL_NO_TYPEID
#  elif defined(__CUDA_ARCH__)
#    define _CCCL_NO_TYPEID // No typeid in CUDA device code
#  elif _CCCL_COMPILER(NVRTC)
#    define _CCCL_NO_TYPEID
#  elif _CCCL_COMPILER(MSVC)
// No-op, MSVC always supports typeid even when RTTI is disabled
#  elif _CCCL_COMPILER(CLANG)
#    if !_CCCL_HAS_FEATURE(cxx_rtti)
#      define _CCCL_NO_TYPEID
#    endif
#  else
#    if __GXX_RTTI == 0 && __cpp_rtti == 0
#      define _CCCL_NO_TYPEID
#    endif
#  endif
#endif // !_CCCL_NO_TYPEID

#endif // __CCCL_RTTI_H
