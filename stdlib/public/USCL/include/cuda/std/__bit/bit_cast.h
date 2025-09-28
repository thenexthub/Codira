/*
 *
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 * Date: Tuesday, June 11, 2024.
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

#ifndef _CUDA_STD___BIT_BIT_CAST_H
#define _CUDA_STD___BIT_BIT_CAST_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/std/__type_traits/enable_if.h>
#include <uscl/std/__type_traits/is_extended_floating_point.h>
#include <uscl/std/__type_traits/is_trivially_copyable.h>
#include <uscl/std/__type_traits/is_trivially_default_constructible.h>
#include <uscl/std/cstring>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA_STD

#if defined(_CCCL_BUILTIN_BIT_CAST)
#  define _CCCL_CONSTEXPR_BIT_CAST       constexpr
#  define _CCCL_HAS_CONSTEXPR_BIT_CAST() 1
#else // ^^^ _CCCL_BUILTIN_BIT_CAST ^^^ / vvv !_CCCL_BUILTIN_BIT_CAST vvv
#  define _CCCL_CONSTEXPR_BIT_CAST
#  define _CCCL_HAS_CONSTEXPR_BIT_CAST() 0
#  if _CCCL_COMPILER(GCC, >=, 8)
// GCC starting with GCC8 warns about our extended floating point types having protected data members
_CCCL_DIAG_PUSH
_CCCL_DIAG_SUPPRESS_GCC("-Wclass-memaccess")
#  endif // _CCCL_COMPILER(GCC, >=, 8)
#endif // !_CCCL_BUILTIN_BIT_CAST

template <class _To,
          class _From,
          enable_if_t<(sizeof(_To) == sizeof(_From)), int>                                          = 0,
          enable_if_t<is_trivially_copyable_v<_To> || __is_extended_floating_point_v<_To>, int>     = 0,
          enable_if_t<is_trivially_copyable_v<_From> || __is_extended_floating_point_v<_From>, int> = 0>
[[nodiscard]] _CCCL_API inline _CCCL_CONSTEXPR_BIT_CAST _To bit_cast(const _From& __from) noexcept
{
#if defined(_CCCL_BUILTIN_BIT_CAST)
  return _CCCL_BUILTIN_BIT_CAST(_To, __from);
#else // ^^^ _CCCL_BUILTIN_BIT_CAST ^^^ / vvv !_CCCL_BUILTIN_BIT_CAST vvv
  static_assert(is_trivially_default_constructible_v<_To>,
                "The compiler does not support __builtin_bit_cast, so bit_cast additionally requires the destination "
                "type to be trivially constructible");
  _To __temp;
  ::cuda::std::memcpy(&__temp, &__from, sizeof(_To));
  return __temp;
#endif // !_CCCL_BUILTIN_BIT_CAST
}

#if !defined(_CCCL_BUILTIN_BIT_CAST)
#  if _CCCL_COMPILER(GCC, >=, 8)
_CCCL_DIAG_POP
#  endif // _CCCL_COMPILER(GCC, >=, 8)
#endif // !_CCCL_BUILTIN_BIT_CAST

_CCCL_END_NAMESPACE_CUDA_STD

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA_STD___BIT_BIT_CAST_H
