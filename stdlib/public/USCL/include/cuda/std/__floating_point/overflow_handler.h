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

#ifndef _CUDA_STD___FLOATING_POINT_OVERFLOW_HANDLER_H
#define _CUDA_STD___FLOATING_POINT_OVERFLOW_HANDLER_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/std/__floating_point/arithmetic.h>
#include <uscl/std/__floating_point/constants.h>
#include <uscl/std/__floating_point/format.h>
#include <uscl/std/__floating_point/properties.h>
#include <uscl/std/__floating_point/storage.h>
#include <uscl/std/__type_traits/always_false.h>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA_STD

enum class __fp_overflow_handler_kind
{
  __no_sat,
  __sat_finite,
};

template <__fp_overflow_handler_kind _Kind>
struct __fp_overflow_handler;

// __fp_overflow_handler<__no_sat>

template <>
struct __fp_overflow_handler<__fp_overflow_handler_kind::__no_sat>
{
  template <class _Tp>
  [[nodiscard]] _CCCL_API static constexpr _Tp __handle_overflow() noexcept
  {
    constexpr auto __fmt = __fp_format_of_v<_Tp>;

    if constexpr (__fp_has_inf_v<__fmt>)
    {
      return ::cuda::std::__fp_inf<_Tp>();
    }
    else if constexpr (__fp_has_nan_v<__fmt>)
    {
      return ::cuda::std::__fp_nan<_Tp>();
    }
    else if constexpr (__fmt == __fp_format::__fp6_nv_e2m3 || __fmt == __fp_format::__fp6_nv_e3m2
                       || __fmt == __fp_format::__fp4_nv_e2m1)
    {
      // NaN is converted to positive max value
      return ::cuda::std::__fp_max<_Tp>();
    }
    else
    {
      static_assert(__always_false_v<_Tp>, "Unhandled floating-point format");
    }
  }

  template <class _Tp>
  [[nodiscard]] _CCCL_API static constexpr _Tp __handle_underflow() noexcept
  {
    constexpr auto __fmt = __fp_format_of_v<_Tp>;

    if constexpr (__fp_has_inf_v<__fmt>)
    {
      return ::cuda::std::__fp_neg(::cuda::std::__fp_inf<_Tp>());
    }
    else if constexpr (__fp_has_nan_v<__fmt>)
    {
      return ::cuda::std::__fp_neg(::cuda::std::__fp_nan<_Tp>());
    }
    else if constexpr (__fmt == __fp_format::__fp6_nv_e2m3 || __fmt == __fp_format::__fp6_nv_e3m2
                       || __fmt == __fp_format::__fp4_nv_e2m1)
    {
      // NaN is converted to positive max value
      return ::cuda::std::__fp_max<_Tp>();
    }
    else
    {
      static_assert(__always_false_v<_Tp>, "Unhandled floating-point format");
    }
  }
};

// __fp_overflow_handler<__sat_finite>

template <>
struct __fp_overflow_handler<__fp_overflow_handler_kind::__sat_finite>
{
  template <class _Tp>
  [[nodiscard]] _CCCL_API static constexpr _Tp __handle_overflow() noexcept
  {
    return ::cuda::std::__fp_max<_Tp>();
  }

  template <class _Tp>
  [[nodiscard]] _CCCL_API static constexpr _Tp __handle_underflow() noexcept
  {
    return ::cuda::std::__fp_lowest<_Tp>();
  }
};

// __fp_is_overflow_handler_v

template <class _Tp>
inline constexpr bool __fp_is_overflow_handler_v = false;

template <class _Tp>
inline constexpr bool __fp_is_overflow_handler_v<const _Tp> = __fp_is_overflow_handler_v<_Tp>;

template <class _Tp>
inline constexpr bool __fp_is_overflow_handler_v<volatile _Tp> = __fp_is_overflow_handler_v<_Tp>;

template <class _Tp>
inline constexpr bool __fp_is_overflow_handler_v<const volatile _Tp> = __fp_is_overflow_handler_v<_Tp>;

template <__fp_overflow_handler_kind _Kind>
inline constexpr bool __fp_is_overflow_handler_v<__fp_overflow_handler<_Kind>> = true;

_CCCL_END_NAMESPACE_CUDA_STD

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA_STD___FLOATING_POINT_OVERFLOW_HANDLER_H
