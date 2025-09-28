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

#ifndef _CUDA___BIT_BITFILED_INSERT_EXTRACT_H
#define _CUDA___BIT_BITFILED_INSERT_EXTRACT_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/__bit/bitmask.h>
#include <uscl/std/__limits/numeric_limits.h>
#include <uscl/std/__type_traits/conditional.h>
#include <uscl/std/__type_traits/is_constant_evaluated.h>
#include <uscl/std/__type_traits/is_unsigned_integer.h>
#include <uscl/std/cstdint>
#include <uscl/std/limits>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA

#if __cccl_ptx_isa >= 200

[[nodiscard]] _CCCL_HIDE_FROM_ABI _CCCL_DEVICE uint32_t
__bfi(uint32_t __dest, uint32_t __source, int __start, int __width) noexcept
{
  asm("bfi.b32 %0, %1, %2, %3, %4;" : "=r"(__dest) : "r"(__source), "r"(__dest), "r"(__start), "r"(__width));
  return __dest;
}

[[nodiscard]] _CCCL_HIDE_FROM_ABI _CCCL_DEVICE uint64_t
__bfi(uint64_t __dest, uint64_t __source, int __start, int __width) noexcept
{
  asm("bfi.b64 %0, %1, %2, %3, %4;" : "=l"(__dest) : "l"(__source), "l"(__dest), "r"(__start), "r"(__width));
  return __dest;
}

[[nodiscard]] _CCCL_HIDE_FROM_ABI _CCCL_DEVICE uint32_t __bfe(uint32_t __value, int __start, int __width) noexcept
{
  uint32_t __ret;
  asm("bfe.u32 %0, %1, %2, %3;" : "=r"(__ret) : "r"(__value), "r"(__start), "r"(__width));
  return __ret;
}

[[nodiscard]] _CCCL_HIDE_FROM_ABI _CCCL_DEVICE uint64_t __bfe(uint64_t __value, int __start, int __width) noexcept
{
  uint64_t __ret;
  asm("bfe.u64 %0, %1, %2, %3;" : "=l"(__ret) : "l"(__value), "r"(__start), "r"(__width));
  return __ret;
}

#endif // __cccl_ptx_isa >= 200

template <typename _Tp>
[[nodiscard]] _CCCL_API constexpr _Tp
bitfield_insert(const _Tp __dest, const _Tp __source, int __start, int __width) noexcept
{
  static_assert(::cuda::std::__cccl_is_cv_unsigned_integer_v<_Tp>, "bitfield_insert() requires unsigned integer types");
  [[maybe_unused]] constexpr auto __digits = ::cuda::std::numeric_limits<_Tp>::digits;
  _CCCL_ASSERT(__width >= 0 && __width <= __digits, "width out of range");
  _CCCL_ASSERT(__start >= 0 && __start <= __digits, "start position out of range");
  _CCCL_ASSERT(__start + __width <= __digits, "start position + width out of range");
  if constexpr (sizeof(_Tp) <= sizeof(uint64_t))
  {
    if (!::cuda::std::__cccl_default_is_constant_evaluated())
    {
      // clang-format off
      NV_DISPATCH_TARGET( // all SM < 70
        NV_PROVIDES_SM_70, (;),
        NV_IS_DEVICE,      (using _Up = ::cuda::std::_If<sizeof(_Tp) <= sizeof(uint32_t), uint32_t, uint64_t>;
                            return ::cuda::__bfi(static_cast<_Up>(__dest), static_cast<_Up>(__source),
                                                 __start, __width);))
      // clang-format on
    }
  }
  auto __mask = ::cuda::bitmask<_Tp>(__start, __width);
  return (::cuda::__shl(__source, __start) & __mask) | (__dest & ~__mask);
}

template <typename _Tp>
[[nodiscard]] _CCCL_API constexpr _Tp bitfield_extract(const _Tp __value, int __start, int __width) noexcept
{
  static_assert(::cuda::std::__cccl_is_cv_unsigned_integer_v<_Tp>,
                "bitfield_extract() requires unsigned integer types");
  [[maybe_unused]] constexpr auto __digits = ::cuda::std::numeric_limits<_Tp>::digits;
  _CCCL_ASSERT(__width >= 0 && __width <= __digits, "width out of range");
  _CCCL_ASSERT(__start >= 0 && __start <= __digits, "start position out of range");
  _CCCL_ASSERT(__start + __width <= __digits, "start position + width out of range");
  if constexpr (sizeof(_Tp) <= sizeof(uint32_t))
  {
    if (!::cuda::std::__cccl_default_is_constant_evaluated())
    {
      // clang-format off
      NV_DISPATCH_TARGET( // all SM < 70
        NV_PROVIDES_SM_70, (;),
        NV_IS_DEVICE,      (using _Up = ::cuda::std::_If<sizeof(_Tp) <= sizeof(uint32_t), uint32_t, uint64_t>;
                            return ::cuda::__bfe(static_cast<_Up>(__value), __start, __width);))
      // clang-format on
    }
  }
  return ::cuda::__shr(__value, __start) & ::cuda::bitmask<_Tp>(0, __width);
}

_CCCL_END_NAMESPACE_CUDA

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA___BIT_BITFILED_INSERT_EXTRACT_H
