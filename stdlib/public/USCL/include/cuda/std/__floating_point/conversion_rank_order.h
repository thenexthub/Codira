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

#ifndef _CUDA_STD___FLOATING_POINT_CONVERSION_RANK_ORDER_H
#define _CUDA_STD___FLOATING_POINT_CONVERSION_RANK_ORDER_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/std/__concepts/concept_macros.h>
#include <uscl/std/__floating_point/traits.h>
#include <uscl/std/__type_traits/conditional.h>
#include <uscl/std/__type_traits/is_integral.h>
#include <uscl/std/__type_traits/is_same.h>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA_STD

enum class __fp_conv_rank_order
{
  __invalid = -1,
  __unordered,
  __greater,
  __equal,
  __less,
};

template <class _Lhs, class _Rhs>
[[nodiscard]] _CCCL_API _CCCL_CONSTEVAL __fp_conv_rank_order __fp_conv_rank_order_v_impl() noexcept
{
  if constexpr (__is_fp_v<_Lhs> && __is_fp_v<_Rhs>)
  {
    if constexpr (__fp_is_subset_of_v<_Lhs, _Rhs> && __fp_is_subset_of_v<_Rhs, _Lhs>)
    {
#if _CCCL_HAS_LONG_DOUBLE()
      // If double and long double have the same properties, long double has the higher subrank
      if constexpr (__fp_is_subset_of_v<long double, double>)
      {
        if constexpr (is_same_v<_Lhs, long double> && !is_same_v<_Rhs, long double>)
        {
          return __fp_conv_rank_order::__greater;
        }
        else if constexpr (!is_same_v<_Lhs, long double> && is_same_v<_Rhs, long double>)
        {
          return __fp_conv_rank_order::__less;
        }
        else
        {
          return __fp_conv_rank_order::__equal;
        }
      }
      else
#endif // _CCCL_HAS_LONG_DOUBLE()
      {
        return __fp_conv_rank_order::__equal;
      }
    }
    else if constexpr (__fp_is_subset_of_v<_Rhs, _Lhs>)
    {
      return __fp_conv_rank_order::__greater;
    }
    else if constexpr (__fp_is_subset_of_v<_Lhs, _Rhs>)
    {
      return __fp_conv_rank_order::__less;
    }
    else
    {
      return __fp_conv_rank_order::__unordered;
    }
  }
  else
  {
    return __fp_conv_rank_order::__invalid;
  }
}

//! @brief Returns the conversion rank order between two types. If any of the types is not a known floating point type,
//!        returns __fp_conv_rank_order::__invalid.
template <class _Lhs, class _Rhs>
inline constexpr __fp_conv_rank_order __fp_conv_rank_order_v = ::cuda::std::__fp_conv_rank_order_v_impl<_Lhs, _Rhs>();

//! @brief Returns the conversion rank order between two types. Integral types are treated as `double`. Other types are
//!        treated as unknown and return __fp_conv_rank_order::__invalid.
template <class _Lhs, class _Rhs>
inline constexpr __fp_conv_rank_order __fp_conv_rank_order_int_ext_v =
  __fp_conv_rank_order_v<conditional_t<is_integral_v<_Lhs>, double, _Lhs>,
                         conditional_t<is_integral_v<_Rhs>, double, _Rhs>>;

//! @brief True if _From can be implicitly converted to _To according to the floating point conversion rank rules.
//! @warning User should ensure that the types are known floating point types.
//! @note If you want to check for explicit conversions, use __fp_is_explicit_conversion_v instead.
template <class _From, class _To>
inline constexpr bool __fp_is_implicit_conversion_v =
  __fp_conv_rank_order_v<_From, _To> == __fp_conv_rank_order::__less
  || __fp_conv_rank_order_v<_From, _To> == __fp_conv_rank_order::__equal;

//! @brief True if _From can be explicitly converted to _To according to the floating point conversion rank rules.
//! @warning User should ensure that the types are known floating point types.
//! @note If you want to check for implicit conversions, use __fp_is_implicit_conversion_v instead.
template <class _From, class _To>
inline constexpr bool __fp_is_explicit_conversion_v =
  __fp_conv_rank_order_v<_From, _To> == __fp_conv_rank_order::__greater
  || __fp_conv_rank_order_v<_From, _To> == __fp_conv_rank_order::__unordered;

_CCCL_END_NAMESPACE_CUDA_STD

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA_STD___FLOATING_POINT_CONVERSION_RANK_ORDER_H
