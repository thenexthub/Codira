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

#ifndef _CUDA___CMATH_ISQRT_H
#define _CUDA___CMATH_ISQRT_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/std/__bit/integral.h>
#include <uscl/std/__concepts/concept_macros.h>
#include <uscl/std/__type_traits/is_integer.h>
#include <uscl/std/__type_traits/is_signed.h>
#include <uscl/std/__type_traits/make_unsigned.h>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA

//! @brief Returns the square root of the given non-negative integer rounded down
//! @param __v The input number
//! @pre \p __v must be an integer type
//! @pre \p __v must be non-negative
//! @return The square root of \p __v rounded down
//! @warning If \p __v is negative, the behavior is undefined
_CCCL_TEMPLATE(class _Tp)
_CCCL_REQUIRES(::cuda::std::__cccl_is_integer_v<_Tp>)
[[nodiscard]] _CCCL_API constexpr _Tp isqrt(_Tp __v) noexcept
{
  if constexpr (::cuda::std::is_signed_v<_Tp>)
  {
    _CCCL_ASSERT(__v >= _Tp{0}, "cuda::isqrt requires non-negative input");
  }

  if (__v <= 1)
  {
    return __v;
  }

  using _Up = ::cuda::std::make_unsigned_t<_Tp>;

  _Up __uv = static_cast<_Up>(__v);
  _Up __ret{};
  _Up __bit = static_cast<_Up>(_Up{1} << ((::cuda::std::bit_width(__uv) - 1) & (~1)));

  while (__bit != 0)
  {
    if (__uv >= __ret + __bit)
    {
      __uv -= __ret + __bit;
      __ret = (__ret >> 1) + __bit;
    }
    else
    {
      __ret >>= 1;
    }
    __bit >>= 2;
  }
  return static_cast<_Tp>(__ret);
}

_CCCL_END_NAMESPACE_CUDA

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA___CMATH_ISQRT_H
