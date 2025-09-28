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

#ifndef _CUDA_STD___MEMORY_USES_ALLOCATOR_H
#define _CUDA_STD___MEMORY_USES_ALLOCATOR_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/std/__type_traits/is_convertible.h>
#include <uscl/std/__type_traits/void_t.h>
#include <uscl/std/cstddef>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA_STD

template <class _Tp, class = void>
inline constexpr bool __has_allocator_type_v = false;
template <class _Tp>
inline constexpr bool __has_allocator_type_v<_Tp, void_t<typename _Tp::allocator_type>> = true;

template <class _Tp, class _Alloc, bool = __has_allocator_type_v<_Tp>>
inline constexpr bool __uses_allocator_v = false;
template <class _Tp, class _Alloc>
inline constexpr bool __uses_allocator_v<_Tp, _Alloc, true> = is_convertible_v<_Alloc, typename _Tp::allocator_type>;

template <class _Tp, class _Alloc>
struct _CCCL_TYPE_VISIBILITY_DEFAULT uses_allocator : public integral_constant<bool, __uses_allocator_v<_Tp, _Alloc>>
{};

template <class _Tp, class _Alloc>
inline constexpr bool uses_allocator_v = __uses_allocator_v<_Tp, _Alloc>;

_CCCL_END_NAMESPACE_CUDA_STD

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA_STD___MEMORY_USES_ALLOCATOR_H
