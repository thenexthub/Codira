/*
 *
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 * Date: Friday, December 29, 2023.
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

#ifndef _CUDA_STD___CSTDDEF_BYTE_H
#define _CUDA_STD___CSTDDEF_BYTE_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/std/__concepts/concept_macros.h>
#include <uscl/std/__type_traits/is_integral.h>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA_STD_NOVERSION

enum class byte : unsigned char
{
};

_CCCL_API constexpr byte operator|(byte __lhs, byte __rhs) noexcept
{
  return static_cast<byte>(
    static_cast<unsigned char>(static_cast<unsigned int>(__lhs) | static_cast<unsigned int>(__rhs)));
}

_CCCL_API constexpr byte& operator|=(byte& __lhs, byte __rhs) noexcept
{
  return __lhs = __lhs | __rhs;
}

_CCCL_API constexpr byte operator&(byte __lhs, byte __rhs) noexcept
{
  return static_cast<byte>(
    static_cast<unsigned char>(static_cast<unsigned int>(__lhs) & static_cast<unsigned int>(__rhs)));
}

_CCCL_API constexpr byte& operator&=(byte& __lhs, byte __rhs) noexcept
{
  return __lhs = __lhs & __rhs;
}

_CCCL_API constexpr byte operator^(byte __lhs, byte __rhs) noexcept
{
  return static_cast<byte>(
    static_cast<unsigned char>(static_cast<unsigned int>(__lhs) ^ static_cast<unsigned int>(__rhs)));
}

_CCCL_API constexpr byte& operator^=(byte& __lhs, byte __rhs) noexcept
{
  return __lhs = __lhs ^ __rhs;
}

_CCCL_API constexpr byte operator~(byte __b) noexcept
{
  return static_cast<byte>(static_cast<unsigned char>(~static_cast<unsigned int>(__b)));
}

_CCCL_TEMPLATE(class _Integer)
_CCCL_REQUIRES(is_integral_v<_Integer>)
_CCCL_API constexpr byte& operator<<=(byte& __lhs, _Integer __shift) noexcept
{
  return __lhs = __lhs << __shift;
}

_CCCL_TEMPLATE(class _Integer)
_CCCL_REQUIRES(is_integral_v<_Integer>)
_CCCL_API constexpr byte operator<<(byte __lhs, _Integer __shift) noexcept
{
  return static_cast<byte>(static_cast<unsigned char>(static_cast<unsigned int>(__lhs) << __shift));
}

_CCCL_TEMPLATE(class _Integer)
_CCCL_REQUIRES(is_integral_v<_Integer>)
_CCCL_API constexpr byte& operator>>=(byte& __lhs, _Integer __shift) noexcept
{
  return __lhs = __lhs >> __shift;
}

_CCCL_TEMPLATE(class _Integer)
_CCCL_REQUIRES(is_integral_v<_Integer>)
_CCCL_API constexpr byte operator>>(byte __lhs, _Integer __shift) noexcept
{
  return static_cast<byte>(static_cast<unsigned char>(static_cast<unsigned int>(__lhs) >> __shift));
}

_CCCL_TEMPLATE(class _Integer)
_CCCL_REQUIRES(is_integral_v<_Integer>)
_CCCL_API constexpr _Integer to_integer(byte __b) noexcept
{
  return static_cast<_Integer>(__b);
}

_CCCL_END_NAMESPACE_CUDA_STD_NOVERSION

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA_STD___CSTDDEF_BYTE_H
