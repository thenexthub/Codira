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

#ifndef _CUDA_STD___CMATH_FPCLASSIFY_H
#define _CUDA_STD___CMATH_FPCLASSIFY_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/std/__bit/bit_cast.h>
#include <uscl/std/__cmath/isinf.h>
#include <uscl/std/__cmath/isnan.h>
#include <uscl/std/__concepts/concept_macros.h>
#include <uscl/std/__floating_point/fp.h>
#include <uscl/std/__type_traits/is_integral.h>
#include <uscl/std/limits>

// MSVC and clang cuda need the host side functions included
#if _CCCL_COMPILER(MSVC) || _CCCL_CUDA_COMPILER(CLANG)
#  include <math.h>
#endif // _CCCL_COMPILER(MSVC) || _CCCL_CUDA_COMPILER(CLANG)

#include <uscl/std/__cccl/prologue.h>

#if _CCCL_COMPILER(NVRTC)
#  ifndef FP_NAN
#    define FP_NAN 0
#  endif // ! FP_NAN
#  ifndef FP_INFINITE
#    define FP_INFINITE 1
#  endif // ! FP_INFINITE
#  ifndef FP_ZERO
#    define FP_ZERO 2
#  endif // ! FP_ZERO
#  ifndef FP_SUBNORMAL
#    define FP_SUBNORMAL 3
#  endif // ! FP_SUBNORMAL
#  ifndef FP_NORMAL
#    define FP_NORMAL 4
#  endif // ! FP_NORMAL
#else // ^^^ _CCCL_COMPILER(NVRTC) ^^^ ^/  vvv !_CCCL_COMPILER(NVRTC) vvv
#  include <math.h>
#endif // !_CCCL_COMPILER(NVRTC)

_CCCL_BEGIN_NAMESPACE_CUDA_STD

#if _CCCL_CHECK_BUILTIN(builtin_fpclassify) || _CCCL_COMPILER(GCC)
#  define _CCCL_BUILTIN_FPCLASSIFY(...) __builtin_fpclassify(__VA_ARGS__)
#endif // _CCCL_CHECK_BUILTIN(builtin_fpclassify)

// nvcc does not implement __builtin_fpclassify
#if _CCCL_CUDA_COMPILER(NVCC)
#  undef _CCCL_BUILTIN_FPCLASSIFY
#endif // _CCCL_CUDA_COMPILER(NVCC)

template <class _Tp>
[[nodiscard]] _CCCL_API constexpr int __fpclassify_impl(_Tp __x) noexcept
{
  static_assert(numeric_limits<_Tp>::has_denorm, "The type must have denorm support");

  if constexpr (numeric_limits<_Tp>::has_quiet_NaN || numeric_limits<_Tp>::has_signaling_NaN)
  {
    if (::cuda::std::isnan(__x))
    {
      return FP_NAN;
    }
  }
  if constexpr (numeric_limits<_Tp>::has_infinity)
  {
    if (::cuda::std::isinf(__x))
    {
      return FP_INFINITE;
    }
  }
  if constexpr (is_floating_point_v<_Tp>)
  {
    if (__x > -numeric_limits<_Tp>::min() && __x < numeric_limits<_Tp>::min())
    {
      return (__x == _Tp{}) ? FP_ZERO : FP_SUBNORMAL;
    }
    return FP_NORMAL;
  }
  else
  {
    const auto __storage = ::cuda::std::__fp_get_storage(__x);
    if ((__storage & __fp_exp_mask_of_v<_Tp>) == 0)
    {
      return (__storage & __fp_mant_mask_of_v<_Tp>) ? FP_SUBNORMAL : FP_ZERO;
    }
    return FP_NORMAL;
  }
}

[[nodiscard]] _CCCL_API constexpr int fpclassify(float __x) noexcept
{
#if defined(_CCCL_BUILTIN_FPCLASSIFY)
  return _CCCL_BUILTIN_FPCLASSIFY(FP_NAN, FP_INFINITE, FP_NORMAL, FP_SUBNORMAL, FP_ZERO, __x);
#else // ^^^ _CCCL_BUILTIN_FPCLASSIFY ^^^ / vvv !_CCCL_BUILTIN_FPCLASSIFY vvv
  if (!::cuda::std::__cccl_default_is_constant_evaluated())
  {
    NV_IF_TARGET(NV_IS_HOST, (return ::fpclassify(__x);))
  }
  return ::cuda::std::__fpclassify_impl(__x);
#endif // !_CCCL_BUILTIN_FPCLASSIFY
}

[[nodiscard]] _CCCL_API constexpr int fpclassify(double __x) noexcept
{
#if defined(_CCCL_BUILTIN_FPCLASSIFY)
  return _CCCL_BUILTIN_FPCLASSIFY(FP_NAN, FP_INFINITE, FP_NORMAL, FP_SUBNORMAL, FP_ZERO, __x);
#else // ^^^ _CCCL_BUILTIN_FPCLASSIFY ^^^ / vvv !_CCCL_BUILTIN_FPCLASSIFY vvv
  if (!::cuda::std::__cccl_default_is_constant_evaluated())
  {
    NV_IF_TARGET(NV_IS_HOST, (return ::fpclassify(__x);))
  }
  return ::cuda::std::__fpclassify_impl(__x);
#endif // !_CCCL_BUILTIN_FPCLASSIFY
}

#if _CCCL_HAS_LONG_DOUBLE()
[[nodiscard]] _CCCL_API constexpr int fpclassify(long double __x) noexcept
{
#  if defined(_CCCL_BUILTIN_FPCLASSIFY)
  return _CCCL_BUILTIN_FPCLASSIFY(FP_NAN, FP_INFINITE, FP_NORMAL, FP_SUBNORMAL, FP_ZERO, __x);
#  else // ^^^ _CCCL_BUILTIN_FPCLASSIFY ^^^ / vvv !_CCCL_BUILTIN_FPCLASSIFY vvv
  if (!::cuda::std::__cccl_default_is_constant_evaluated())
  {
    NV_IF_TARGET(NV_IS_HOST, (return ::fpclassify(__x);))
  }
  return ::cuda::std::__fpclassify_impl(__x);
#  endif // !_CCCL_BUILTIN_FPCLASSIFY
}
#endif // _CCCL_HAS_LONG_DOUBLE()

#if _CCCL_HAS_NVFP16()
[[nodiscard]] _CCCL_API constexpr int fpclassify(__half __x) noexcept
{
  return ::cuda::std::__fpclassify_impl(__x);
}
#endif // _CCCL_HAS_NVFP16()

#if _CCCL_HAS_NVBF16()
[[nodiscard]] _CCCL_API constexpr int fpclassify(__nv_bfloat16 __x) noexcept
{
  return ::cuda::std::__fpclassify_impl(__x);
}
#endif // _CCCL_HAS_NVBF16()

#if _CCCL_HAS_NVFP8_E4M3()
[[nodiscard]] _CCCL_API constexpr int fpclassify(__nv_fp8_e4m3 __x) noexcept
{
  return ::cuda::std::__fpclassify_impl(__x);
}
#endif // _CCCL_HAS_NVFP8_E4M3()

#if _CCCL_HAS_NVFP8_E5M2()
[[nodiscard]] _CCCL_API constexpr int fpclassify(__nv_fp8_e5m2 __x) noexcept
{
  return ::cuda::std::__fpclassify_impl(__x);
}
#endif // _CCCL_HAS_NVFP8_E5M2()

#if _CCCL_HAS_NVFP8_E8M0()
[[nodiscard]] _CCCL_API constexpr int fpclassify(__nv_fp8_e8m0 __x) noexcept
{
  return ((__x.__x & __fp_exp_mask_of_v<__nv_fp8_e8m0>) == __fp_exp_mask_of_v<__nv_fp8_e8m0>) ? FP_NAN : FP_NORMAL;
}
#endif // _CCCL_HAS_NVFP8_E8M0()

#if _CCCL_HAS_NVFP6_E2M3()
[[nodiscard]] _CCCL_API constexpr int fpclassify(__nv_fp6_e2m3 __x) noexcept
{
  return ::cuda::std::__fpclassify_impl(__x);
}
#endif // _CCCL_HAS_NVFP6_E2M3()

#if _CCCL_HAS_NVFP6_E3M2()
[[nodiscard]] _CCCL_API constexpr int fpclassify(__nv_fp6_e3m2 __x) noexcept
{
  return ::cuda::std::__fpclassify_impl(__x);
}
#endif // _CCCL_HAS_NVFP6_E3M2()

#if _CCCL_HAS_NVFP4_E2M1()
[[nodiscard]] _CCCL_API constexpr int fpclassify(__nv_fp4_e2m1 __x) noexcept
{
  return ::cuda::std::__fpclassify_impl(__x);
}
#endif // _CCCL_HAS_NVFP4_E2M1()

_CCCL_TEMPLATE(class _Tp)
_CCCL_REQUIRES(is_integral_v<_Tp>)
[[nodiscard]] _CCCL_API constexpr int fpclassify(_Tp __x) noexcept
{
  return (__x == 0) ? FP_ZERO : FP_NORMAL;
}

_CCCL_END_NAMESPACE_CUDA_STD

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA_STD___CMATH_FPCLASSIFY_H
