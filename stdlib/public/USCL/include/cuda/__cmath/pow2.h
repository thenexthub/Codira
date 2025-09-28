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

#ifndef _CUDA___CMATH_POW2_H
#define _CUDA___CMATH_POW2_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/std/__bit/has_single_bit.h>
#include <uscl/std/__bit/integral.h>
#include <uscl/std/__type_traits/is_integer.h>
#include <uscl/std/__type_traits/is_signed.h>
#include <uscl/std/__type_traits/make_unsigned.h>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA

_CCCL_TEMPLATE(typename _Tp)
_CCCL_REQUIRES(::cuda::std::__cccl_is_integer_v<_Tp>)
[[nodiscard]] _CCCL_API constexpr bool is_power_of_two(_Tp __t) noexcept
{
  if constexpr (::cuda::std::is_signed_v<_Tp>)
  {
    _CCCL_ASSERT(__t >= _Tp{0}, "cuda::is_power_of_two requires non-negative input");
  }
  using _Up = ::cuda::std::make_unsigned_t<_Tp>;
  return ::cuda::std::has_single_bit(static_cast<_Up>(__t));
}

_CCCL_TEMPLATE(typename _Tp)
_CCCL_REQUIRES(::cuda::std::__cccl_is_integer_v<_Tp>)
[[nodiscard]] _CCCL_API constexpr _Tp next_power_of_two(_Tp __t) noexcept
{
  if constexpr (::cuda::std::is_signed_v<_Tp>)
  {
    _CCCL_ASSERT(__t >= _Tp{0}, "cuda::is_power_of_two requires non-negative input");
  }
  using _Up = ::cuda::std::make_unsigned_t<_Tp>;
  return ::cuda::std::bit_ceil(static_cast<_Up>(__t));
}

_CCCL_TEMPLATE(typename _Tp)
_CCCL_REQUIRES(::cuda::std::__cccl_is_integer_v<_Tp>)
[[nodiscard]] _CCCL_API constexpr _Tp prev_power_of_two(_Tp __t) noexcept
{
  if constexpr (::cuda::std::is_signed_v<_Tp>)
  {
    _CCCL_ASSERT(__t >= _Tp{0}, "cuda::is_power_of_two requires non-negative input");
  }
  using _Up = ::cuda::std::make_unsigned_t<_Tp>;
  return ::cuda::std::bit_floor(static_cast<_Up>(__t));
}

_CCCL_END_NAMESPACE_CUDA

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA___CMATH_POW2_H
