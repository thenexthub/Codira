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

#ifndef _CUDA_STD___TYPE_TRAITS_IS_POINTER_INTERCONVERTIBLE_BASE_OF_H
#define _CUDA_STD___TYPE_TRAITS_IS_POINTER_INTERCONVERTIBLE_BASE_OF_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/std/__type_traits/integral_constant.h>
#include <uscl/std/__type_traits/is_class.h>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA_STD

#if defined(_CCCL_BUILTIN_IS_POINTER_INTERCONVERTIBLE_BASE_OF)

template <class _Tp, class _Up>
inline constexpr bool is_pointer_interconvertible_base_of_v =
  _CCCL_BUILTIN_IS_POINTER_INTERCONVERTIBLE_BASE_OF(_Tp, _Up);

#  if _CCCL_COMPILER(CLANG)
// clang's builtin evaluates is_pointer_interconvertible_base_of_v<T, T> to be false which is not right
template <class _Tp>
inline constexpr bool is_pointer_interconvertible_base_of_v<_Tp, _Tp> = is_class_v<_Tp>;
template <class _Tp>
inline constexpr bool is_pointer_interconvertible_base_of_v<_Tp, const _Tp> = is_class_v<_Tp>;
template <class _Tp>
inline constexpr bool is_pointer_interconvertible_base_of_v<_Tp, volatile _Tp> = is_class_v<_Tp>;
template <class _Tp>
inline constexpr bool is_pointer_interconvertible_base_of_v<_Tp, const volatile _Tp> = is_class_v<_Tp>;
template <class _Tp>
inline constexpr bool is_pointer_interconvertible_base_of_v<const _Tp, _Tp> = is_class_v<_Tp>;
template <class _Tp>
inline constexpr bool is_pointer_interconvertible_base_of_v<const _Tp, const _Tp> = is_class_v<_Tp>;
template <class _Tp>
inline constexpr bool is_pointer_interconvertible_base_of_v<const _Tp, volatile _Tp> = is_class_v<_Tp>;
template <class _Tp>
inline constexpr bool is_pointer_interconvertible_base_of_v<const _Tp, const volatile _Tp> = is_class_v<_Tp>;
template <class _Tp>
inline constexpr bool is_pointer_interconvertible_base_of_v<volatile _Tp, _Tp> = is_class_v<_Tp>;
template <class _Tp>
inline constexpr bool is_pointer_interconvertible_base_of_v<volatile _Tp, const _Tp> = is_class_v<_Tp>;
template <class _Tp>
inline constexpr bool is_pointer_interconvertible_base_of_v<volatile _Tp, volatile _Tp> = is_class_v<_Tp>;
template <class _Tp>
inline constexpr bool is_pointer_interconvertible_base_of_v<volatile _Tp, const volatile _Tp> = is_class_v<_Tp>;
template <class _Tp>
inline constexpr bool is_pointer_interconvertible_base_of_v<const volatile _Tp, _Tp> = is_class_v<_Tp>;
template <class _Tp>
inline constexpr bool is_pointer_interconvertible_base_of_v<const volatile _Tp, const _Tp> = is_class_v<_Tp>;
template <class _Tp>
inline constexpr bool is_pointer_interconvertible_base_of_v<const volatile _Tp, volatile _Tp> = is_class_v<_Tp>;
template <class _Tp>
inline constexpr bool is_pointer_interconvertible_base_of_v<const volatile _Tp, const volatile _Tp> = is_class_v<_Tp>;
#  endif // _CCCL_COMPILER(CLANG)

template <class _Tp, class _Up>
struct _CCCL_TYPE_VISIBILITY_DEFAULT
is_pointer_interconvertible_base_of : bool_constant<is_pointer_interconvertible_base_of_v<_Tp, _Up>>
{};

#endif // _CCCL_BUILTIN_IS_POINTER_INTERCONVERTIBLE_BASE_OF

_CCCL_END_NAMESPACE_CUDA_STD

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA_STD___TYPE_TRAITS_IS_POINTER_INTERCONVERTIBLE_BASE_OF_H
