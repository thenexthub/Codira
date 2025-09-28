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

#ifndef _CUDA___BIT_BITMASK_H
#define _CUDA___BIT_BITMASK_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#if _CCCL_CUDA_COMPILATION()
#  include <cuda/__ptx/instructions/bmsk.h>
#  include <cuda/__ptx/instructions/shl.h>
#  include <cuda/__ptx/instructions/shr.h>
#endif // _CCCL_CUDA_COMPILATION()
#include <uscl/std/__type_traits/conditional.h>
#include <uscl/std/__type_traits/is_constant_evaluated.h>
#include <uscl/std/__type_traits/is_unsigned_integer.h>
#include <uscl/std/limits>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA

template <typename _Tp>
[[nodiscard]] _CCCL_API constexpr _Tp __shl(const _Tp __value, int __shift) noexcept
{
  if (!::cuda::std::__cccl_default_is_constant_evaluated())
  {
    if constexpr (sizeof(_Tp) <= sizeof(uint64_t))
    {
      NV_DISPATCH_TARGET(NV_IS_DEVICE,
                         (using _Up = ::cuda::std::_If<sizeof(_Tp) <= sizeof(uint32_t), uint32_t, uint64_t>;
                          return ::cuda::ptx::shl(static_cast<_Up>(__value), __shift);))
    }
  }
  return (__shift >= ::cuda::std::numeric_limits<_Tp>::digits) ? _Tp{0} : __value << __shift;
}

template <typename _Tp>
[[nodiscard]] _CCCL_API constexpr _Tp __shr(const _Tp __value, int __shift) noexcept
{
  if (!::cuda::std::__cccl_default_is_constant_evaluated())
  {
    if constexpr (sizeof(_Tp) <= sizeof(uint64_t))
    {
      NV_DISPATCH_TARGET(NV_IS_DEVICE,
                         (using _Up = ::cuda::std::_If<sizeof(_Tp) <= sizeof(uint32_t), uint32_t, uint64_t>;
                          return ::cuda::ptx::shr(static_cast<_Up>(__value), __shift);))
    }
  }
  return (__shift >= ::cuda::std::numeric_limits<_Tp>::digits) ? _Tp{0} : __value >> __shift;
}

template <typename _Tp = uint32_t>
[[nodiscard]] _CCCL_API constexpr _Tp bitmask(int __start, int __width) noexcept
{
  static_assert(::cuda::std::__cccl_is_unsigned_integer_v<_Tp>, "bitmask() requires unsigned integer types");
  [[maybe_unused]] constexpr auto __digits = ::cuda::std::numeric_limits<_Tp>::digits;
  _CCCL_ASSERT(__width >= 0 && __width <= __digits, "width out of range");
  _CCCL_ASSERT(__start >= 0 && __start <= __digits, "start position out of range");
  _CCCL_ASSERT(__start + __width <= __digits, "start position + width out of range");
  if (!::cuda::std::__cccl_default_is_constant_evaluated())
  {
    if constexpr (sizeof(_Tp) <= sizeof(uint32_t))
    {
      NV_IF_TARGET(NV_PROVIDES_SM_70, (return ::cuda::ptx::bmsk_clamp(__start, __width);))
    }
  }
  return ::cuda::__shl(static_cast<_Tp>(::cuda::__shl(_Tp{1}, __width) - 1), __start);
}

_CCCL_END_NAMESPACE_CUDA

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA___BIT_BITMASK_H
