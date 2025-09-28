/*
 *
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 * Date: Wednesday, July 24, 2024.
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

// IMPORTANT: This file intionally lacks a header guard.

#include <uscl/std/detail/__config>

#if defined(_CUDAX_ASYNC_PROLOGUE_INCLUDED)
#  error multiple inclusion of prologue.cuh
#endif

#define _CUDAX_ASYNC_PROLOGUE_INCLUDED

#include <uscl/std/__cccl/prologue.h>

_CCCL_DIAG_PUSH
_CCCL_DIAG_SUPPRESS_GCC("-Wsubobject-linkage")
_CCCL_DIAG_SUPPRESS_CLANG("-Wunused-value")
_CCCL_DIAG_SUPPRESS_MSVC(4848) // [[no_unique_address]] prior to C++20 as a vendor extension

_CCCL_DIAG_SUPPRESS_GCC("-Wmissing-braces")
_CCCL_DIAG_SUPPRESS_CLANG("-Wmissing-braces")
_CCCL_DIAG_SUPPRESS_MSVC(5246) // missing braces around initializer

#if _CCCL_CUDA_COMPILER(NVHPC)
_CCCL_BEGIN_NV_DIAG_SUPPRESS(cuda_compile)
#endif // _CCCL_CUDA_COMPILER(NVHPC)

// private and protected nested class types cannot be used as tparams to __global__
// functions. _CUDAX_SEMI_PRIVATE expands to public when _CCCL_CUDA_COMPILATION() is true,
// and private otherwise.
#if _CCCL_CUDA_COMPILATION()
#  define _CUDAX_SEMI_PRIVATE public
#else // ^^^ _CCCL_CUDA_COMPILATION() ^^^ / vvv !_CCCL_CUDA_COMPILATION() vvv
#  define _CUDAX_SEMI_PRIVATE private
#endif
