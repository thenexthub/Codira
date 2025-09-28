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

#ifndef _CUDA___CMATH_ILOG_H
#define _CUDA___CMATH_ILOG_H

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
#include <uscl/std/__cmath/rounding_functions.h>
#include <uscl/std/__concepts/concept_macros.h>
#include <uscl/std/__limits/numeric_limits.h>
#include <uscl/std/__type_traits/is_constant_evaluated.h>
#include <uscl/std/__type_traits/is_integer.h>
#include <uscl/std/__type_traits/is_same.h>
#include <uscl/std/__type_traits/make_unsigned.h>
#include <uscl/std/array>
#include <uscl/std/cstdint>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA

_CCCL_TEMPLATE(typename _Tp)
_CCCL_REQUIRES(::cuda::std::__cccl_is_cv_integer_v<_Tp>)
_CCCL_API constexpr int ilog2(_Tp __t) noexcept
{
  using _Up = ::cuda::std::make_unsigned_t<_Tp>;
  _CCCL_ASSERT(__t > 0, "ilog2() argument must be strictly positive");
  auto __log2_approx = ::cuda::std::__bit_log2(static_cast<_Up>(__t));
  _CCCL_ASSUME(__log2_approx <= ::cuda::std::numeric_limits<_Tp>::digits);
  return __log2_approx;
}

_CCCL_TEMPLATE(typename _Tp)
_CCCL_REQUIRES(::cuda::std::__cccl_is_cv_integer_v<_Tp>)
_CCCL_API constexpr int ceil_ilog2(_Tp __t) noexcept
{
  using _Up = ::cuda::std::make_unsigned_t<_Tp>;
  return ::cuda::ilog2(__t) + !::cuda::std::has_single_bit(static_cast<_Up>(__t));
}

[[nodiscard]] _CCCL_API constexpr ::cuda::std::array<uint32_t, 10> __power_of_10_32bit() noexcept
{
  return {10,
          100,
          1'000,
          10'000,
          100'000,
          1'000'000,
          10'000'000,
          100'000'000,
          1'000'000'000,
          ::cuda::std::numeric_limits<uint32_t>::max()};
}

[[nodiscard]] _CCCL_API constexpr ::cuda::std::array<uint64_t, 20> __power_of_10_64bit() noexcept
{
  return {
    10,
    100,
    1'000,
    10'000,
    100'000,
    1'000'000,
    10'000'000,
    100'000'000,
    1'000'000'000,
    10'000'000'000,
    100'000'000'000,
    1'000'000'000'000,
    10'000'000'000'000,
    100'000'000'000'000,
    1'000'000'000'000'000,
    10'000'000'000'000'000,
    100'000'000'000'000'000,
    1'000'000'000'000'000'000,
    10'000'000'000'000'000'000ull,
    ::cuda::std::numeric_limits<uint64_t>::max()};
}

#if _CCCL_HAS_INT128()

[[nodiscard]] _CCCL_API constexpr ::cuda::std::array<__uint128_t, 39> __power_of_10_128bit() noexcept
{
  return {
    10,
    100,
    1'000,
    10'000,
    100'000,
    1'000'000,
    10'000'000,
    100'000'000,
    1'000'000'000,
    10'000'000'000,
    100'000'000'000,
    1'000'000'000'000,
    10'000'000'000'000,
    100'000'000'000'000,
    1'000'000'000'000'000,
    10'000'000'000'000'000,
    100'000'000'000'000'000,
    1'000'000'000'000'000'000,
    10'000'000'000'000'000'000ull,
    __uint128_t{10'000'000'000'000'000'000ull} * 10,
    __uint128_t{10'000'000'000'000'000'000ull} * 100,
    __uint128_t{10'000'000'000'000'000'000ull} * 1'000,
    __uint128_t{10'000'000'000'000'000'000ull} * 10'000,
    __uint128_t{10'000'000'000'000'000'000ull} * 100'000,
    __uint128_t{10'000'000'000'000'000'000ull} * 1'000'000,
    __uint128_t{10'000'000'000'000'000'000ull} * 10'000'000,
    __uint128_t{10'000'000'000'000'000'000ull} * 100'000'000,
    __uint128_t{10'000'000'000'000'000'000ull} * 1'000'000'000,
    __uint128_t{10'000'000'000'000'000'000ull} * 10'000'000'000,
    __uint128_t{10'000'000'000'000'000'000ull} * 100'000'000'000,
    __uint128_t{10'000'000'000'000'000'000ull} * 1'000'000'000'000,
    __uint128_t{10'000'000'000'000'000'000ull} * 10'000'000'000'000,
    __uint128_t{10'000'000'000'000'000'000ull} * 100'000'000'000'000,
    __uint128_t{10'000'000'000'000'000'000ull} * 1'000'000'000'000'000,
    __uint128_t{10'000'000'000'000'000'000ull} * 1'000'000'000'000'0000,
    __uint128_t{10'000'000'000'000'000'000ull} * 10'000'000'000'000'0000,
    __uint128_t{10'000'000'000'000'000'000ull} * 100'000'000'000'000'0000,
    __uint128_t{10'000'000'000'000'000'000ull} * 1'000'000'000'000'000'0000ull,
    ::cuda::std::numeric_limits<__uint128_t>::max()};
}
#endif // _CCCL_HAS_INT128()

_CCCL_TEMPLATE(typename _Tp)
_CCCL_REQUIRES(::cuda::std::__cccl_is_cv_integer_v<_Tp>)
_CCCL_API constexpr int ilog10(_Tp __t) noexcept
{
  _CCCL_ASSERT(__t > 0, "ilog10() argument must be strictly positive");
  constexpr auto __reciprocal_log2_10 = 0.301029995663f; // 1 / log2(10)
  auto __log2                         = ::cuda::ilog2(__t) * __reciprocal_log2_10;
  auto __log10_approx                 = static_cast<int>(__log2);
  if constexpr (sizeof(_Tp) <= sizeof(uint32_t))
  {
    _CCCL_ASSERT(__log10_approx < static_cast<int>(::cuda::__power_of_10_32bit().size()), "out of bounds");
    if constexpr (::cuda::std::is_same_v<_Tp, uint32_t>)
    {
      // don't replace +1 with >= because wraparound behavior is needed here
      __log10_approx += static_cast<uint32_t>(__t) + 1 > ::cuda::__power_of_10_32bit()[__log10_approx];
    }
    else
    {
      __log10_approx += static_cast<uint32_t>(__t) >= ::cuda::__power_of_10_32bit()[__log10_approx];
    }
  }
  else if constexpr (sizeof(_Tp) == sizeof(uint64_t))
  {
    _CCCL_ASSERT(__log10_approx < static_cast<int>(::cuda::__power_of_10_64bit().size()), "out of bounds");
    // +1 is not needed here
    __log10_approx += static_cast<uint64_t>(__t) >= ::cuda::__power_of_10_64bit()[__log10_approx];
  }
#if _CCCL_HAS_INT128()
  else
  {
    _CCCL_ASSERT(__log10_approx < static_cast<int>(::cuda::__power_of_10_128bit().size()), "out of bounds");
    if constexpr (::cuda::std::is_same_v<_Tp, __uint128_t>)
    {
      // don't replace +1 with >= because wraparound behavior is needed here
      __log10_approx += static_cast<__uint128_t>(__t) + 1 > ::cuda::__power_of_10_128bit()[__log10_approx];
    }
    else
    {
      __log10_approx += static_cast<__uint128_t>(__t) >= ::cuda::__power_of_10_128bit()[__log10_approx];
    }
  }
#endif // _CCCL_HAS_INT128()
  _CCCL_ASSUME(__log10_approx <= ::cuda::std::numeric_limits<_Tp>::digits / 3); // 2^X < 10^(x/3) -> 8^X < 10^x
  return __log10_approx;
}

_CCCL_END_NAMESPACE_CUDA

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA___CMATH_ILOG_H
