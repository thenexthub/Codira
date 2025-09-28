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

#ifndef _CUDA___CMATH_IPOW_H
#define _CUDA___CMATH_IPOW_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/__cmath/ilog.h>
#include <uscl/__cmath/uabs.h>
#include <uscl/std/__bit/countl.h>
#include <uscl/std/__bit/has_single_bit.h>
#include <uscl/std/__concepts/concept_macros.h>
#include <uscl/std/__type_traits/is_integer.h>
#include <uscl/std/__type_traits/is_unsigned.h>
#include <uscl/std/__type_traits/make_unsigned.h>
#include <uscl/std/__utility/cmp.h>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA

template <class _Tp, class _Ep>
[[nodiscard]] _CCCL_API constexpr _Tp __cccl_ipow_impl_base_pow2(_Tp __b, _Ep __e) noexcept
{
  const auto __shift = static_cast<int>(__e - 1) * ::cuda::ilog2(__b);
  const auto __lz    = ::cuda::std::countl_zero(__b);
  return (__shift >= __lz) ? _Tp{0} : (_Tp{__b} << __shift);
}

template <class _Tp, class _Ep>
[[nodiscard]] _CCCL_API constexpr _Tp __cccl_ipow_impl(_Tp __b, _Ep __e) noexcept
{
  static_assert(::cuda::std::is_unsigned_v<_Tp>);

  if (::cuda::std::has_single_bit(__b))
  {
    return ::cuda::__cccl_ipow_impl_base_pow2(__b, __e);
  }

  auto __x = __b;
  auto __y = _Tp{1};

  while (__e > 1)
  {
    if (__e % 2 == 1)
    {
      __y *= __x;
      --__e;
    }
    __x *= __x;
    __e /= 2;
  }
  return __x * __y;
}

//! @brief Computes the integer power of a base to an exponent.
//! @param __b The base
//! @param __e The exponent
//! @pre \p __b must be an integer type
//! @pre \p __e must be an integer type
//! @return The result of raising \p __b to the power of \p __e
//! @note The result is undefined if \p __b is 0 and \p __e is negative.
_CCCL_TEMPLATE(class _Tp, class _Ep)
_CCCL_REQUIRES(::cuda::std::__cccl_is_integer_v<_Tp> _CCCL_AND ::cuda::std::__cccl_is_integer_v<_Ep>)
[[nodiscard]] _CCCL_API constexpr _Tp ipow(_Tp __b, _Ep __e) noexcept
{
  _CCCL_ASSERT(__b != _Tp{0} || ::cuda::std::cmp_greater_equal(__e, _Ep{0}),
               "cuda::ipow() requires non-negative exponent for base 0");

  if (__e == _Ep{0} || __b == _Tp{1})
  {
    return _Tp{1};
  }
  else if (::cuda::std::cmp_less(__e, _Ep{0}) || __b == _Tp{0})
  {
    return _Tp{0};
  }
  auto __res = ::cuda::__cccl_ipow_impl(::cuda::uabs(__b), ::cuda::std::__to_unsigned_like(__e));
  if (::cuda::std::cmp_less(__b, _Tp{0}) && (__e % 2u == 1))
  {
    // todo: replace with ::cuda::__neg(__res) when available
    __res = (~__res + 1);
  }
  return static_cast<_Tp>(__res);
}

_CCCL_END_NAMESPACE_CUDA

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA___CMATH_IPOW_H
