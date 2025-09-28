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

#ifndef _CUDA_STD___BIT_INTEGRAL_H
#define _CUDA_STD___BIT_INTEGRAL_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#if _CCCL_CUDA_COMPILATION()
#  include <cuda/__ptx/instructions/bfind.h>
#  include <cuda/__ptx/instructions/shl.h>
#  include <cuda/__ptx/instructions/shr.h>
#endif // _CCCL_CUDA_COMPILATION()
#include <uscl/std/__algorithm/max.h>
#include <uscl/std/__bit/countl.h>
#include <uscl/std/__concepts/concept_macros.h>
#include <uscl/std/__type_traits/conditional.h>
#include <uscl/std/__type_traits/is_constant_evaluated.h>
#include <uscl/std/__type_traits/is_unsigned_integer.h>
#include <uscl/std/cstdint>
#include <uscl/std/limits>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA_STD

template <class _Tp>
_CCCL_API constexpr uint32_t __bit_log2(_Tp __t) noexcept
{
  if (!::cuda::std::__cccl_default_is_constant_evaluated())
  {
    if constexpr (sizeof(_Tp) <= 8)
    {
      using _Up [[maybe_unused]] = _If<sizeof(_Tp) <= 4, uint32_t, uint64_t>;
      NV_IF_TARGET(NV_IS_DEVICE, (return ::cuda::ptx::bfind(static_cast<_Up>(__t));))
    }
    else
    {
      NV_IF_TARGET(NV_IS_DEVICE,
                   (auto __high = ::cuda::ptx::bfind(static_cast<uint64_t>(__t >> 64));
                    return __high == ~uint32_t{0} ? ::cuda::ptx::bfind(static_cast<uint64_t>(__t)) : __high + 64;))
    }
  }
  return numeric_limits<_Tp>::digits - 1 - ::cuda::std::countl_zero(__t);
}

_CCCL_TEMPLATE(class _Tp)
_CCCL_REQUIRES(::cuda::std::__cccl_is_unsigned_integer_v<_Tp>)
[[nodiscard]] _CCCL_API constexpr int bit_width(_Tp __t) noexcept
{
  // if __t == 0, __bit_log2(0) returns 0xFFFFFFFF. Since unsigned overflow is well-defined, the result is -1 + 1 = 0
  auto __ret = ::cuda::std::__bit_log2(__t) + 1;
  _CCCL_ASSUME(__ret <= numeric_limits<_Tp>::digits);
  return static_cast<int>(__ret);
}

_CCCL_TEMPLATE(class _Tp)
_CCCL_REQUIRES(::cuda::std::__cccl_is_unsigned_integer_v<_Tp>)
[[nodiscard]] _CCCL_API constexpr _Tp bit_ceil(_Tp __t) noexcept
{
  using _Up = _If<sizeof(_Tp) <= 4, uint32_t, _Tp>;
  _CCCL_ASSERT(__t <= numeric_limits<_Tp>::max() / 2, "bit_ceil overflow");
  // if __t == 0, __t - 1 == 0xFFFFFFFF, bit_width(0xFFFFFFFF) returns 32
  auto __width = ::cuda::std::bit_width(static_cast<_Up>(__t) - 1);
  if constexpr (sizeof(_Tp) <= 8)
  {
    if (!::cuda::std::__cccl_default_is_constant_evaluated())
    {
      // CUDA right shift (ptx::shr) returns 0 if the right operand is larger than the number of bits of the type
      // The result is computed as max(1, bit_width(__t - 1)) because it is more efficient than the ternary operator
      NV_IF_TARGET(NV_IS_DEVICE, //
                   (auto __shift = ::cuda::ptx::shl(_Up{1}, __width); // 2^(ceil(log2(__t - 1)))
                    auto __ret   = static_cast<_Tp>(::cuda::std::max(_Up{1}, __shift)); //
                    _CCCL_ASSUME(__ret >= __t);
                    return __ret;))
    }
  }
  auto __ret = static_cast<_Tp>(__t <= 1 ? _Up{1} : _Up{1} << __width);
  _CCCL_ASSUME(__ret >= __t);
  return __ret;
}

_CCCL_TEMPLATE(class _Tp)
_CCCL_REQUIRES(::cuda::std::__cccl_is_unsigned_integer_v<_Tp>)
[[nodiscard]] _CCCL_API constexpr _Tp bit_floor(_Tp __t) noexcept
{
  using _Up   = _If<sizeof(_Tp) <= 4, uint32_t, _Tp>;
  auto __log2 = ::cuda::std::__bit_log2(static_cast<_Up>(__t));
  // __bit_log2 returns 0xFFFFFFFF if __t == 0
  if constexpr (sizeof(_Tp) <= 8)
  {
    if (!::cuda::std::__cccl_default_is_constant_evaluated())
    {
      // CUDA left shift (ptx::shl) returns 0 if the right operand is larger than the number of bits of the type
      // -> the result is 0 if __t == 0
      NV_IF_TARGET(NV_IS_DEVICE, //
                   (auto __ret = static_cast<_Tp>(::cuda::ptx::shl(_Up{1}, __log2)); // 2^(log2(t))
                    _CCCL_ASSUME(__ret >= __t / 2 && __ret <= __t);
                    return __ret;))
    }
  }
  auto __ret = static_cast<_Tp>(__t == 0 ? _Up{0} : _Up{1} << __log2);
  _CCCL_ASSUME(__ret >= __t / 2 && __ret <= __t);
  return __ret;
}

_CCCL_END_NAMESPACE_CUDA_STD

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA_STD___BIT_INTEGRAL_H
