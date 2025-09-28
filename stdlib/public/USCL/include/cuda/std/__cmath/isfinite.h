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

#ifndef _CUDA_STD___CMATH_ISFINITE_H
#define _CUDA_STD___CMATH_ISFINITE_H

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

// MSVC and clang cuda need the host side functions included
#if _CCCL_COMPILER(MSVC) || _CCCL_CUDA_COMPILER(CLANG)
#  include <math.h>
#endif // _CCCL_COMPILER(MSVC) || _CCCL_CUDA_COMPILER(CLANG)

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA_STD

#if _CCCL_CHECK_BUILTIN(builtin_isfinite) || _CCCL_COMPILER(GCC) || _CCCL_COMPILER(NVRTC, >, 12, 2)
#  define _CCCL_BUILTIN_ISFINITE(...) __builtin_isfinite(__VA_ARGS__)
#endif // _CCCL_CHECK_BUILTIN(isfinite)

template <class _Tp>
[[nodiscard]] _CCCL_API constexpr bool __isfinite_impl(_Tp __x) noexcept
{
  static_assert(is_floating_point_v<_Tp>, "Only standard floating-point types are supported");
  if (!::cuda::std::__cccl_default_is_constant_evaluated())
  {
    return ::isfinite(__x);
  }
  return !::cuda::std::isnan(__x) && !::cuda::std::isinf(__x);
}

[[nodiscard]] _CCCL_API constexpr bool isfinite(float __x) noexcept
{
#if defined(_CCCL_BUILTIN_ISFINITE)
  return _CCCL_BUILTIN_ISFINITE(__x);
#else // ^^^ _CCCL_BUILTIN_ISFINITE ^^^ / vvv !_CCCL_BUILTIN_ISFINITE vvv
  if (!::cuda::std::__cccl_default_is_constant_evaluated())
  {
    return ::isfinite(__x);
  }
#  if _CCCL_HAS_CONSTEXPR_BIT_CAST()
  return (::cuda::std::__fp_get_storage(__x) & __fp_exp_mask_of_v<float>) != __fp_exp_mask_of_v<float>;
#  else // ^^^ _CCCL_HAS_CONSTEXPR_BIT_CAST() ^^^ / vvv !_CCCL_HAS_CONSTEXPR_BIT_CAST() vvv
  return ::cuda::std::__isfinite_impl(__x);
#  endif // ^^^ !_CCCL_HAS_CONSTEXPR_BIT_CAST() ^^^
#endif // ^^^ !_CCCL_BUILTIN_ISFINITE ^^^
}

[[nodiscard]] _CCCL_API constexpr bool isfinite(double __x) noexcept
{
#if defined(_CCCL_BUILTIN_ISFINITE)
  return _CCCL_BUILTIN_ISFINITE(__x);
#else // ^^^ _CCCL_BUILTIN_ISFINITE ^^^ / vvv !_CCCL_BUILTIN_ISFINITE vvv
  if (!::cuda::std::__cccl_default_is_constant_evaluated())
  {
    return ::isfinite(__x);
  }
#  if _CCCL_HAS_CONSTEXPR_BIT_CAST()
  return (::cuda::std::__fp_get_storage(__x) & __fp_exp_mask_of_v<double>) != __fp_exp_mask_of_v<double>;
#  else // ^^^ _CCCL_HAS_CONSTEXPR_BIT_CAST() ^^^ / vvv !_CCCL_HAS_CONSTEXPR_BIT_CAST() vvv
  return ::cuda::std::__isfinite_impl(__x);
#  endif // ^^^ !_CCCL_HAS_CONSTEXPR_BIT_CAST() ^^^
#endif // ^^^ !_CCCL_BUILTIN_ISFINITE ^^^
}

#if _CCCL_HAS_LONG_DOUBLE()
[[nodiscard]] _CCCL_API constexpr bool isfinite(long double __x) noexcept
{
#  if defined(_CCCL_BUILTIN_ISFINITE)
  return _CCCL_BUILTIN_ISFINITE(__x);
#  else
  return ::cuda::std::__isfinite_impl(__x);
#  endif // defined(_CCCL_BUILTIN_ISFINITE)
}
#endif // _CCCL_HAS_LONG_DOUBLE()

#if _CCCL_HAS_NVFP16()
[[nodiscard]] _CCCL_API constexpr bool isfinite(__half __x) noexcept
{
  return (::cuda::std::__fp_get_storage(__x) & __fp_exp_mask_of_v<__half>) != __fp_exp_mask_of_v<__half>;
}
#endif // _CCCL_HAS_NVFP16()

#if _CCCL_HAS_NVBF16()
[[nodiscard]] _CCCL_API constexpr bool isfinite(__nv_bfloat16 __x) noexcept
{
  return (::cuda::std::__fp_get_storage(__x) & __fp_exp_mask_of_v<__nv_bfloat16>) != __fp_exp_mask_of_v<__nv_bfloat16>;
}
#endif // _CCCL_HAS_NVBF16()

#if _CCCL_HAS_NVFP8_E4M3()
[[nodiscard]] _CCCL_API constexpr bool isfinite(__nv_fp8_e4m3 __x) noexcept
{
  return (__x.__x & __fp_exp_mant_mask_of_v<__nv_fp8_e4m3>) != __fp_exp_mant_mask_of_v<__nv_fp8_e4m3>;
}
#endif // _CCCL_HAS_NVFP8_E4M3()

#if _CCCL_HAS_NVFP8_E5M2()
[[nodiscard]] _CCCL_API constexpr bool isfinite(__nv_fp8_e5m2 __x) noexcept
{
  return (__x.__x & __fp_exp_mask_of_v<__nv_fp8_e5m2>) != __fp_exp_mask_of_v<__nv_fp8_e5m2>;
}
#endif // _CCCL_HAS_NVFP8_E5M2()

#if _CCCL_HAS_NVFP8_E8M0()
[[nodiscard]] _CCCL_API constexpr bool isfinite(__nv_fp8_e8m0 __x) noexcept
{
  return __x.__x != __fp_exp_mask_of_v<__nv_fp8_e8m0>;
}
#endif // _CCCL_HAS_NVFP8_E8M0()

#if _CCCL_HAS_NVFP6_E2M3()
[[nodiscard]] _CCCL_API constexpr bool isfinite(__nv_fp6_e2m3) noexcept
{
  return true;
}
#endif // _CCCL_HAS_NVFP6_E2M3()

#if _CCCL_HAS_NVFP6_E3M2()
[[nodiscard]] _CCCL_API constexpr bool isfinite(__nv_fp6_e3m2) noexcept
{
  return true;
}
#endif // _CCCL_HAS_NVFP6_E3M2()

#if _CCCL_HAS_NVFP4_E2M1()
[[nodiscard]] _CCCL_API constexpr bool isfinite(__nv_fp4_e2m1) noexcept
{
  return true;
}
#endif // _CCCL_HAS_NVFP4_E2M1()

_CCCL_TEMPLATE(class _Tp)
_CCCL_REQUIRES(is_integral_v<_Tp>)
[[nodiscard]] _CCCL_API constexpr bool isfinite(_Tp) noexcept
{
  return true;
}

_CCCL_END_NAMESPACE_CUDA_STD

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA_STD___CMATH_ISFINITE_H
