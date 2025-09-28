/*
 *
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 * Date: Friday, December 29, 2023.
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

#ifndef _CUDA_PTX_CP_REDUCE_ASYNC_BULK_H_
#define _CUDA_PTX_CP_REDUCE_ASYNC_BULK_H_

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/__ptx/ptx_dot_variants.h>
#include <uscl/__ptx/ptx_helper_functions.h>
#include <uscl/std/cstdint>

#include <nv/target> // __CUDA_MINIMUM_ARCH__ and friends

// Forward-declare __half and __nv_bfloat16. The cuda_fp16.h and cuda_bf16.h are
// expensive to include. The APIs use only pointers, so we do not have to define
// the types. If the user wants to use these types, it is their responsibility
// to include the headers.
#if _LIBCUDACXX_HAS_NVFP16()
struct __half;
#endif // _LIBCUDACXX_HAS_NVFP16()
#if _LIBCUDACXX_HAS_NVBF16()
struct __nv_bfloat16;
#endif // _LIBCUDACXX_HAS_NVBF16()

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA_PTX

// 9.7.8.24.7. Data Movement and Conversion Instructions: cp.reduce.async.bulk
// https://docs.nvidia.com/cuda/parallel-thread-execution/index.html#data-movement-and-conversion-instructions-cp-reduce-async-bulk
#include <uscl/__ptx/instructions/generated/cp_reduce_async_bulk.h>
#if _LIBCUDACXX_HAS_NVFP16()
#  include <cuda/__ptx/instructions/generated/cp_reduce_async_bulk_f16.h>
#endif // _LIBCUDACXX_HAS_NVFP16()
#if _LIBCUDACXX_HAS_NVBF16()
#  include <cuda/__ptx/instructions/generated/cp_reduce_async_bulk_bf16.h>
#endif // _LIBCUDACXX_HAS_NVBF16()

_CCCL_END_NAMESPACE_CUDA_PTX

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA_PTX_CP_REDUCE_ASYNC_BULK_H_
