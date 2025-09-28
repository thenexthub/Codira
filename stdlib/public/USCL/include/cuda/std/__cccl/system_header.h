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

#ifndef __CCCL_SYSTEM_HEADER_H
#define __CCCL_SYSTEM_HEADER_H

#include <uscl/std/__cccl/compiler.h>
#include <uscl/std/__cccl/is_non_narrowing_convertible.h> // IWYU pragma: export

// Enforce that cccl headers are treated as system headers
#if _CCCL_COMPILER(GCC) || _CCCL_COMPILER(NVHPC)
#  define _CCCL_FORCE_SYSTEM_HEADER_GCC
#elif _CCCL_COMPILER(CLANG)
#  define _CCCL_FORCE_SYSTEM_HEADER_CLANG
#elif _CCCL_COMPILER(MSVC)
#  define _CCCL_FORCE_SYSTEM_HEADER_MSVC
#endif // other compilers

// Potentially enable that cccl headers are treated as system headers
#if !defined(_CCCL_NO_SYSTEM_HEADER) && !(_CCCL_COMPILER(MSVC) && defined(_LIBCUDACXX_DISABLE_PRAGMA_MSVC_WARNING)) \
  && !_CCCL_COMPILER(NVRTC) && !defined(_LIBCUDACXX_DISABLE_PRAGMA_GCC_SYSTEM_HEADER)
#  if _CCCL_COMPILER(GCC) || _CCCL_COMPILER(NVHPC)
#    define _CCCL_IMPLICIT_SYSTEM_HEADER_GCC
#  elif _CCCL_COMPILER(CLANG)
#    define _CCCL_IMPLICIT_SYSTEM_HEADER_CLANG
#  elif _CCCL_COMPILER(MSVC)
#    define _CCCL_IMPLICIT_SYSTEM_HEADER_MSVC
#  endif // other compilers
#endif // Use system header

#endif // __CCCL_SYSTEM_HEADER_H
