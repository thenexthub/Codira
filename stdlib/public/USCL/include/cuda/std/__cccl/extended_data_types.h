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

#ifndef __CCCL_EXTENDED_DATA_TYPES_H
#define __CCCL_EXTENDED_DATA_TYPES_H

#include <uscl/std/__cccl/compiler.h>
#include <uscl/std/__cccl/system_header.h>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/std/__cccl/architecture.h>
#include <uscl/std/__cccl/cuda_toolkit.h>
#include <uscl/std/__cccl/diagnostic.h>
#include <uscl/std/__cccl/execution_space.h>
#include <uscl/std/__cccl/os.h>
#include <uscl/std/__cccl/preprocessor.h>

#define _CCCL_HAS_INT128()      0
#define _CCCL_HAS_LONG_DOUBLE() 0
#define _CCCL_HAS_NVFP4()       0
#define _CCCL_HAS_NVFP6()       0
#define _CCCL_HAS_NVFP8()       0
#define _CCCL_HAS_NVFP16()      0
#define _CCCL_HAS_NVBF16()      0
#define _CCCL_HAS_FLOAT128()    0

#if !defined(CCCL_DISABLE_INT128_SUPPORT) && _CCCL_OS(LINUX) \
  && ((_CCCL_COMPILER(NVRTC) && defined(__CUDACC_RTC_INT128__)) || defined(__SIZEOF_INT128__))
#  undef _CCCL_HAS_INT128
#  define _CCCL_HAS_INT128() 1
#endif

// Fixme: replace the condition with (!_CCCL_DEVICE_COMPILATION())
// FIXME: Enable this for clang-cuda in a followup
#if !_CCCL_HAS_CUDA_COMPILER()
#  undef _CCCL_HAS_LONG_DOUBLE
#  define _CCCL_HAS_LONG_DOUBLE() 1
#endif // !_CCCL_HAS_CUDA_COMPILER()

#if _CCCL_HAS_INCLUDE(<cuda_fp16.h>) && (_CCCL_HAS_CTK() || defined(LIBCUDACXX_ENABLE_HOST_NVFP16)) \
                      && !defined(CCCL_DISABLE_FP16_SUPPORT)
#  undef _CCCL_HAS_NVFP16
#  define _CCCL_HAS_NVFP16() 1
#endif

#if _CCCL_HAS_INCLUDE(<cuda_bf16.h>) && _CCCL_HAS_NVFP16() && !defined(CCCL_DISABLE_BF16_SUPPORT)
#  undef _CCCL_HAS_NVBF16
#  define _CCCL_HAS_NVBF16() 1
#endif

#if _CCCL_HAS_INCLUDE(<cuda_fp8.h>) && _CCCL_HAS_NVFP16() && _CCCL_HAS_NVBF16() && !defined(CCCL_DISABLE_NVFP8_SUPPORT)
#  undef _CCCL_HAS_NVFP8
#  define _CCCL_HAS_NVFP8() 1
#endif

#if _CCCL_HAS_INCLUDE(<cuda_fp6.h>) && _CCCL_HAS_NVFP8() && !_CCCL_CUDA_COMPILER(NVHPC) \
                      && !defined(CCCL_DISABLE_NVFP6_SUPPORT)
#  undef _CCCL_HAS_NVFP6
#  define _CCCL_HAS_NVFP6() 1
#endif

#if _CCCL_HAS_INCLUDE(<cuda_fp4.h>) && _CCCL_HAS_NVFP6() && !defined(CCCL_DISABLE_NVFP4_SUPPORT)
#  undef _CCCL_HAS_NVFP4
#  define _CCCL_HAS_NVFP4() 1
#endif

#define _CCCL_HAS_NVFP4_E2M1() _CCCL_HAS_NVFP4()
#define _CCCL_HAS_NVFP6_E2M3() _CCCL_HAS_NVFP6()
#define _CCCL_HAS_NVFP6_E3M2() _CCCL_HAS_NVFP6()
#define _CCCL_HAS_NVFP8_E4M3() _CCCL_HAS_NVFP8()
#define _CCCL_HAS_NVFP8_E5M2() _CCCL_HAS_NVFP8()
#define _CCCL_HAS_NVFP8_E8M0() (_CCCL_HAS_NVFP8() && _CCCL_CTK_AT_LEAST(12, 8))

/***********************************************************************************************************************
 * __float128
 **********************************************************************************************************************/

#if !defined(CCCL_DISABLE_FLOAT128_SUPPORT) && _CCCL_HAS_INT128() && _CCCL_OS(LINUX) && !_CCCL_ARCH(ARM64)
// Detect host compiler support
#  if (defined(__CUDACC_RTC_FLOAT128__) || defined(__SIZEOF_FLOAT128__) || defined(__FLOAT128__))
#    if _CCCL_DEVICE_COMPILATION()
// Only NVCC and NVRTC 12.8+ on architectures at least SM100 supports __float128 on device
#      if (_CCCL_CUDA_COMPILER(NVCC, >=, 12, 8) || _CCCL_CUDA_COMPILER(NVRTC, >=, 12, 8)) && _CCCL_PTX_ARCH() >= 1000
#        undef _CCCL_HAS_FLOAT128
#        define _CCCL_HAS_FLOAT128() 1
#      endif // _CCCL_CUDA_COMPILER(NVCC) && _CCCL_PTX_ARCH() >= 1000
#    else // ^^^ _CCCL_DEVICE_COMPILATION() ^^^ / vvv !_CCCL_DEVICE_COMPILATION() vvv
#      undef _CCCL_HAS_FLOAT128
#      define _CCCL_HAS_FLOAT128() 1
#    endif // ^^^ !_CCCL_DEVICE_COMPILATION() ^^^
#  endif // Host compiler support
#endif // !defined(CCCL_DISABLE_FLOAT128_SUPPORT) && _CCCL_HAS_INT128() && _CCCL_OS(LINUX) && !_CCCL_ARCH(ARM64)

// gcc does not allow to use q/Q floating point literals when __STRICT_ANSI__ is defined. They may be allowed by
// -fext-numeric-literals, but there is no way to detect it in the preprocessor. The user is required to define
// CCCL_GCC_HAS_EXTENDED_NUMERIC_LITERALS in this case. Otherwise, we disable the __float128 support.
//
// Note: since GCC 13, we could use f128/F128 literals, but for values > DBL_MAX, the compilation with nvcc fails due to
//       "floating constant is out of range".
#if _CCCL_HAS_FLOAT128() && _CCCL_COMPILER(GCC) && defined(__STRICT_ANSI__) \
  && !defined(CCCL_GCC_HAS_EXTENDED_NUMERIC_LITERALS)
#  undef _CCCL_HAS_FLOAT128
#  define _CCCL_HAS_FLOAT128() 0
#endif // _CCCL_HAS_FLOAT128()

/***********************************************************************************************************************
 * char8_t
 **********************************************************************************************************************/

#if _CCCL_STD_VER <= 2017 || !defined(__cpp_char8_t)
#  define _CCCL_HAS_CHAR8_T() 0
#else
#  define _CCCL_HAS_CHAR8_T() 1
#endif // _CCCL_STD_VER <= 2017 || !defined(__cpp_char8_t)

// We currently do not support any of the STL wchar facilities
#define _CCCL_HAS_WCHAR_T() 0

#endif // __CCCL_EXTENDED_DATA_TYPES_H
