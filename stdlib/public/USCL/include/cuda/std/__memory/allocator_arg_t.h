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

#ifndef _CUDA_STD___FUNCTIONAL_ALLOCATOR_ARG_T_H
#define _CUDA_STD___FUNCTIONAL_ALLOCATOR_ARG_T_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/std/__memory/uses_allocator.h>
#include <uscl/std/__type_traits/integral_constant.h>
#include <uscl/std/__type_traits/is_constructible.h>
#include <uscl/std/__type_traits/remove_cvref.h>
#include <uscl/std/__utility/forward.h>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA_STD

struct _CCCL_TYPE_VISIBILITY_DEFAULT allocator_arg_t
{
  _CCCL_HIDE_FROM_ABI explicit allocator_arg_t() = default;
};

inline constexpr allocator_arg_t allocator_arg = allocator_arg_t();

// allocator construction

template <class _Tp, class _Alloc, class... _Args>
struct __uses_alloc_ctor_imp
{
  using _RawAlloc _CCCL_NODEBUG_ALIAS = remove_cvref_t<_Alloc>;
  static const bool __ua              = uses_allocator<_Tp, _RawAlloc>::value;
  static const bool __ic              = is_constructible<_Tp, allocator_arg_t, _Alloc, _Args...>::value;
  static const int value              = __ua ? 2 - __ic : 0;
};

template <class _Tp, class _Alloc, class... _Args>
struct __uses_alloc_ctor : integral_constant<int, __uses_alloc_ctor_imp<_Tp, _Alloc, _Args...>::value>
{};

template <class _Tp, class _Allocator, class... _Args>
_CCCL_API inline void
__user_alloc_construct_impl(integral_constant<int, 0>, _Tp* __storage, const _Allocator&, _Args&&... __args)
{
  new (__storage) _Tp(::cuda::std::forward<_Args>(__args)...);
}

// FIXME: This should have a version which takes a non-const alloc.
template <class _Tp, class _Allocator, class... _Args>
_CCCL_API inline void
__user_alloc_construct_impl(integral_constant<int, 1>, _Tp* __storage, const _Allocator& __a, _Args&&... __args)
{
  new (__storage) _Tp(allocator_arg, __a, ::cuda::std::forward<_Args>(__args)...);
}

// FIXME: This should have a version which takes a non-const alloc.
template <class _Tp, class _Allocator, class... _Args>
_CCCL_API inline void
__user_alloc_construct_impl(integral_constant<int, 2>, _Tp* __storage, const _Allocator& __a, _Args&&... __args)
{
  new (__storage) _Tp(::cuda::std::forward<_Args>(__args)..., __a);
}

_CCCL_END_NAMESPACE_CUDA_STD

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA_STD___FUNCTIONAL_ALLOCATOR_ARG_T_H
