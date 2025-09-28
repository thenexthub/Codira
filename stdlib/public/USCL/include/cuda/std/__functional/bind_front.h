/*
 *
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 * Date: Wednesday, November 8, 2023.
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

#ifndef _CUDA_STD___FUNCTIONAL_BIND_FRONT_H
#define _CUDA_STD___FUNCTIONAL_BIND_FRONT_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/std/__concepts/concept_macros.h>
#include <uscl/std/__functional/invoke.h>
#include <uscl/std/__functional/perfect_forward.h>
#include <uscl/std/__type_traits/decay.h>
#include <uscl/std/__type_traits/enable_if.h>
#include <uscl/std/__type_traits/is_constructible.h>
#include <uscl/std/__type_traits/is_move_constructible.h>
#include <uscl/std/__type_traits/is_nothrow_constructible.h>
#include <uscl/std/__utility/forward.h>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA_STD

struct __bind_front_op
{
  template <class... _Args>
  _CCCL_API constexpr auto operator()(_Args&&... __args) const
    noexcept(noexcept(::cuda::std::invoke(::cuda::std::forward<_Args>(__args)...)))
      -> decltype(::cuda::std::invoke(::cuda::std::forward<_Args>(__args)...))
  {
    return ::cuda::std::invoke(::cuda::std::forward<_Args>(__args)...);
  }
};

template <class _Fn, class... _BoundArgs>
struct __bind_front_t : __perfect_forward<__bind_front_op, _Fn, _BoundArgs...>
{
  _LIBCUDACXX_DELEGATE_CONSTRUCTORS(__bind_front_t, __perfect_forward, __bind_front_op, _Fn, _BoundArgs...);
};

template <class _Fn, class... _Args>
_CCCL_CONCEPT __can_bind_front =
  is_constructible_v<decay_t<_Fn>, _Fn> && is_move_constructible_v<decay_t<_Fn>>
  && (is_constructible_v<decay_t<_Args>, _Args> && ...) && (is_move_constructible_v<decay_t<_Args>> && ...);

_CCCL_TEMPLATE(class _Fn, class... _Args)
_CCCL_REQUIRES(__can_bind_front<_Fn, _Args...>)
_CCCL_API constexpr auto
bind_front(_Fn&& __f, _Args&&... __args) noexcept(is_nothrow_constructible_v<tuple<decay_t<_Args>...>, _Args&&...>)
{
  return __bind_front_t<decay_t<_Fn>, decay_t<_Args>...>(
    ::cuda::std::forward<_Fn>(__f), ::cuda::std::forward<_Args>(__args)...);
}

_CCCL_END_NAMESPACE_CUDA_STD

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA_STD___FUNCTIONAL_BIND_FRONT_H
