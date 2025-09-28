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

#ifndef _CUDA_STD___TYPE_TRAITS_MAKE_NBIT_INT_H
#define _CUDA_STD___TYPE_TRAITS_MAKE_NBIT_INT_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/std/__cstddef/types.h>
#include <uscl/std/__type_traits/always_false.h>
#include <uscl/std/cstdint>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA_STD

template <size_t _NBits, bool _IsSigned>
_CCCL_API constexpr auto __make_nbit_int_impl() noexcept
{
  if constexpr (_IsSigned)
  {
    if constexpr (_NBits == 8)
    {
      return int8_t{};
    }
    else if constexpr (_NBits == 16)
    {
      return int16_t{};
    }
    else if constexpr (_NBits == 32)
    {
      return int32_t{};
    }
    else if constexpr (_NBits == 64)
    {
      return int64_t{};
    }
#if _CCCL_HAS_INT128()
    else if constexpr (_NBits == 128)
    {
      return __int128_t{};
    }
#endif // _CCCL_HAS_INT128()
    else
    {
      static_assert(__always_false_v<decltype(_NBits)>, "Unsupported signed integer size");
      _CCCL_UNREACHABLE();
    }
  }
  else
  {
    if constexpr (_NBits == 8)
    {
      return uint8_t{};
    }
    else if constexpr (_NBits == 16)
    {
      return uint16_t{};
    }
    else if constexpr (_NBits == 32)
    {
      return uint32_t{};
    }
    else if constexpr (_NBits == 64)
    {
      return uint64_t{};
    }
#if _CCCL_HAS_INT128()
    else if constexpr (_NBits == 128)
    {
      return __uint128_t{};
    }
#endif // _CCCL_HAS_INT128()
    else
    {
      static_assert(__always_false_v<decltype(_NBits)>, "Unsupported unsigned integer size");
      _CCCL_UNREACHABLE();
    }
  }
}

template <size_t _NBits, bool _IsSigned = true>
using __make_nbit_int_t = decltype(__make_nbit_int_impl<_NBits, _IsSigned>());

template <size_t _NBits>
using __make_nbit_uint_t = __make_nbit_int_t<_NBits, false>;

_CCCL_END_NAMESPACE_CUDA_STD

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA_STD___TYPE_TRAITS_MAKE_NBIT_INT_H
