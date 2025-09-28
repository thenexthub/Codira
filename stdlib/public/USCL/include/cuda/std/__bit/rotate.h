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

#ifndef _CUDA_STD___BIT_ROTATE_H
#define _CUDA_STD___BIT_ROTATE_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/__cmath/neg.h>
#include <uscl/std/__concepts/concept_macros.h>
#include <uscl/std/__type_traits/is_constant_evaluated.h>
#include <uscl/std/__type_traits/is_same.h>
#include <uscl/std/__type_traits/is_unsigned_integer.h>
#include <uscl/std/cstdint>
#include <uscl/std/limits>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA_STD

template <typename _Tp>
[[nodiscard]] _CCCL_API constexpr _Tp __cccl_rotr_impl(_Tp __v, int __cnt) noexcept
{
  if constexpr (sizeof(_Tp) == sizeof(uint32_t))
  {
    if (!::cuda::std::__cccl_default_is_constant_evaluated())
    {
      NV_IF_TARGET(NV_IS_DEVICE, (return ::__funnelshift_r(__v, __v, __cnt);))
    }
  }
  constexpr auto __digits = numeric_limits<_Tp>::digits;
  auto __cnt_mod          = static_cast<uint32_t>(__cnt) % __digits; // __cnt is always >= 0
  return __cnt_mod == 0 ? __v : (__v >> __cnt_mod) | (__v << (__digits - __cnt_mod));
}

template <typename _Tp>
[[nodiscard]] _CCCL_API constexpr _Tp __cccl_rotl_impl(_Tp __v, int __cnt) noexcept
{
  if constexpr (sizeof(_Tp) == sizeof(uint32_t))
  {
    if (!::cuda::std::__cccl_default_is_constant_evaluated())
    {
      NV_IF_TARGET(NV_IS_DEVICE, (return ::__funnelshift_l(__v, __v, __cnt);))
    }
  }
  constexpr auto __digits = numeric_limits<_Tp>::digits;
  auto __cnt_mod          = static_cast<uint32_t>(__cnt) % __digits; // __cnt is always >= 0
  return __cnt_mod == 0 ? __v : (__v << __cnt_mod) | (__v >> (__digits - __cnt_mod));
}

_CCCL_TEMPLATE(class _Tp)
_CCCL_REQUIRES(::cuda::std::__cccl_is_unsigned_integer_v<_Tp>)
[[nodiscard]] _CCCL_API constexpr _Tp rotl(_Tp __v, int __cnt) noexcept
{
  if (__cnt < 0)
  {
    __cnt = static_cast<int>(static_cast<unsigned>(::cuda::neg(__cnt)) % numeric_limits<_Tp>::digits);
    return ::cuda::std::__cccl_rotr_impl(__v, __cnt);
  }
  return ::cuda::std::__cccl_rotl_impl(__v, __cnt);
}

_CCCL_TEMPLATE(class _Tp)
_CCCL_REQUIRES(::cuda::std::__cccl_is_unsigned_integer_v<_Tp>)
[[nodiscard]] _CCCL_API constexpr _Tp rotr(_Tp __v, int __cnt) noexcept
{
  if (__cnt < 0)
  {
    __cnt = static_cast<int>(static_cast<unsigned>(::cuda::neg(__cnt)) % numeric_limits<_Tp>::digits);
    return ::cuda::std::__cccl_rotl_impl(__v, __cnt);
  }
  return ::cuda::std::__cccl_rotr_impl(__v, __cnt);
}

_CCCL_END_NAMESPACE_CUDA_STD

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA_STD___BIT_ROTATE_H
