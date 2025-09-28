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

#ifndef _CUDA_STD___COMPLEX_ARG_H
#define _CUDA_STD___COMPLEX_ARG_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/std/__cmath/inverse_trigonometric_functions.h>
#include <uscl/std/__complex/complex.h>
#include <uscl/std/__floating_point/cuda_fp_types.h>
#include <uscl/std/__type_traits/is_integral.h>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA_STD

// arg

template <class _Tp>
[[nodiscard]] _CCCL_API inline _Tp arg(const complex<_Tp>& __c)
{
  return ::cuda::std::atan2(__c.imag(), __c.real());
}

[[nodiscard]] _CCCL_API inline float arg(float __re)
{
  return ::cuda::std::atan2f(0.F, __re);
}

[[nodiscard]] _CCCL_API inline double arg(double __re)
{
  return ::cuda::std::atan2(0.0, __re);
}

#if _CCCL_HAS_LONG_DOUBLE()
[[nodiscard]] _CCCL_API inline long double arg(long double __re)
{
  return ::cuda::std::atan2l(0.L, __re);
}
#endif // _CCCL_HAS_LONG_DOUBLE()

#if _LIBCUDACXX_HAS_NVBF16()
[[nodiscard]] _CCCL_API inline __nv_bfloat16 arg(__nv_bfloat16 __re)
{
  return ::cuda::std::atan2(::__int2bfloat16_rn(0), __re);
}
#endif // _LIBCUDACXX_HAS_NVBF16()

#if _LIBCUDACXX_HAS_NVFP16()
[[nodiscard]] _CCCL_API inline __half arg(__half __re)
{
  return ::cuda::std::atan2(::__int2half_rn(0), __re);
}
#endif // _LIBCUDACXX_HAS_NVFP16()

_CCCL_TEMPLATE(class _Tp)
_CCCL_REQUIRES(is_integral_v<_Tp>)
[[nodiscard]] _CCCL_API inline double arg(_Tp __re)
{
  // integrals need to be promoted to double
  return ::cuda::std::arg(static_cast<double>(__re));
}

_CCCL_END_NAMESPACE_CUDA_STD

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA_STD___COMPLEX_MATH_H
