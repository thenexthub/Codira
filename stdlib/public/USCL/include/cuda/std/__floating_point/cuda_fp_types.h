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

#ifndef _CUDA_STD___FLOATING_POINT_NVFP_TYPES_H
#define _CUDA_STD___FLOATING_POINT_NVFP_TYPES_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

// Prevent resetting of the diagnostic state by guarding the push/pop with a macro
#if _CCCL_HAS_NVFP16()
_CCCL_DIAG_PUSH
#  include <cuda_fp16.h>
_CCCL_DIAG_POP
#endif // _CCCL_HAS_NVFP16()

#if _CCCL_HAS_NVBF16()
_CCCL_DIAG_PUSH
_CCCL_DIAG_SUPPRESS_CLANG("-Wunused-function")
#  include <cuda_bf16.h>
_CCCL_DIAG_POP
#endif // _CCCL_HAS_NVBF16()

#if _CCCL_HAS_NVFP8()
_CCCL_DIAG_PUSH
#  include <cuda_fp8.h>
_CCCL_DIAG_POP
#endif // _CCCL_HAS_NVFP8()

#if _CCCL_HAS_NVFP6()
_CCCL_DIAG_PUSH
#  include <cuda_fp6.h>
_CCCL_DIAG_POP
#endif // _CCCL_HAS_NVFP6()

#if _CCCL_HAS_NVFP4()
_CCCL_DIAG_PUSH
_CCCL_DIAG_SUPPRESS_GCC("-Wunused-parameter")
_CCCL_DIAG_SUPPRESS_MSVC(4100) // unreferenced formal parameter
#  include <cuda_fp4.h>
_CCCL_DIAG_POP
#endif // _CCCL_HAS_NVFP4()

#if _CCCL_HAS_FLOAT128() && _CCCL_DEVICE_COMPILATION() && _CCCL_CTK_AT_LEAST(12, 8)
#  if !_CCCL_COMPILER(NVRTC)
_CCCL_DIAG_PUSH
#    include <crt/device_fp128_functions.h>
_CCCL_DIAG_POP
#  else // ^^^ !_CCCL_COMPILER(NVRTC) ^^^ / vvv _CCCL_COMPILER(NVRTC) vvv
__device__ __cudart_builtin__ __float128 __nv_fp128_sqrt(__float128);
__device__ __cudart_builtin__ __float128 __nv_fp128_sin(__float128);
__device__ __cudart_builtin__ __float128 __nv_fp128_cos(__float128);
__device__ __cudart_builtin__ __float128 __nv_fp128_tan(__float128);
__device__ __cudart_builtin__ __float128 __nv_fp128_asin(__float128);
__device__ __cudart_builtin__ __float128 __nv_fp128_acos(__float128);
__device__ __cudart_builtin__ __float128 __nv_fp128_atan(__float128);
__device__ __cudart_builtin__ __float128 __nv_fp128_exp(__float128);
__device__ __cudart_builtin__ __float128 __nv_fp128_exp2(__float128);
__device__ __cudart_builtin__ __float128 __nv_fp128_exp10(__float128);
__device__ __cudart_builtin__ __float128 __nv_fp128_expm1(__float128);
__device__ __cudart_builtin__ __float128 __nv_fp128_log(__float128);
__device__ __cudart_builtin__ __float128 __nv_fp128_log2(__float128);
__device__ __cudart_builtin__ __float128 __nv_fp128_log10(__float128);
__device__ __cudart_builtin__ __float128 __nv_fp128_log1p(__float128);
__device__ __cudart_builtin__ __float128 __nv_fp128_pow(__float128, __float128);
__device__ __cudart_builtin__ __float128 __nv_fp128_sinh(__float128);
__device__ __cudart_builtin__ __float128 __nv_fp128_cosh(__float128);
__device__ __cudart_builtin__ __float128 __nv_fp128_tanh(__float128);
__device__ __cudart_builtin__ __float128 __nv_fp128_asinh(__float128);
__device__ __cudart_builtin__ __float128 __nv_fp128_acosh(__float128);
__device__ __cudart_builtin__ __float128 __nv_fp128_atanh(__float128);
__device__ __cudart_builtin__ __float128 __nv_fp128_trunc(__float128);
__device__ __cudart_builtin__ __float128 __nv_fp128_floor(__float128);
__device__ __cudart_builtin__ __float128 __nv_fp128_ceil(__float128);
__device__ __cudart_builtin__ __float128 __nv_fp128_round(__float128);
__device__ __cudart_builtin__ __float128 __nv_fp128_rint(__float128);
__device__ __cudart_builtin__ __float128 __nv_fp128_fabs(__float128);
__device__ __cudart_builtin__ __float128 __nv_fp128_copysign(__float128, __float128);
__device__ __cudart_builtin__ __float128 __nv_fp128_fmax(__float128, __float128);
__device__ __cudart_builtin__ __float128 __nv_fp128_fmin(__float128, __float128);
__device__ __cudart_builtin__ __float128 __nv_fp128_fdim(__float128, __float128);
__device__ __cudart_builtin__ __float128 __nv_fp128_fmod(__float128, __float128);
__device__ __cudart_builtin__ __float128 __nv_fp128_remainder(__float128, __float128);
__device__ __cudart_builtin__ __float128 __nv_fp128_frexp(__float128, int*);
__device__ __cudart_builtin__ __float128 __nv_fp128_modf(__float128, __float128*);
__device__ __cudart_builtin__ __float128 __nv_fp128_hypot(__float128, __float128);
__device__ __cudart_builtin__ __float128 __nv_fp128_fma(__float128, __float128, __float128);
__device__ __cudart_builtin__ __float128 __nv_fp128_ldexp(__float128, int);
__device__ __cudart_builtin__ int __nv_fp128_ilogb(__float128);
__device__ __cudart_builtin__ __float128 __nv_fp128_mul(__float128, __float128);
__device__ __cudart_builtin__ __float128 __nv_fp128_add(__float128, __float128);
__device__ __cudart_builtin__ __float128 __nv_fp128_sub(__float128, __float128);
__device__ __cudart_builtin__ __float128 __nv_fp128_div(__float128, __float128);
__device__ __cudart_builtin__ int __nv_fp128_isnan(__float128);
__device__ __cudart_builtin__ int __nv_fp128_isunordered(__float128, __float128);
#  endif // ^^^ _CCCL_COMPILER(NVRTC) ^^^
#endif // _CCCL_HAS_FLOAT128() && _CCCL_DEVICE_COMPILATION() && _CCCL_CTK_AT_LEAST(12, 8)

#endif // _CUDA_STD___FLOATING_POINT_NVFP_TYPES_H
