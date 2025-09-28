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

#ifndef _CUDA_STD___FUNCTIONAL_NOT_FN_H
#define _CUDA_STD___FUNCTIONAL_NOT_FN_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/std/__functional/invoke.h>
#include <uscl/std/__type_traits/decay.h>
#include <uscl/std/__type_traits/enable_if.h>
#include <uscl/std/__type_traits/is_constructible.h>
#include <uscl/std/__type_traits/is_move_constructible.h>
#include <uscl/std/__utility/forward.h>
#include <uscl/std/__utility/move.h>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA_STD

template <class _Fn, class... _Args>
_CCCL_CONCEPT __can_invoke_and_negate = _CCCL_REQUIRES_EXPR((_Fn, variadic _Args), _Fn&& __f, _Args&&... __args)(
  (!::cuda::std::invoke(::cuda::std::forward<_Fn>(__f), ::cuda::std::forward<_Args>(__args)...)));

template <class _Fn>
struct __not_fn_t
{
  _Fn __f;

  _CCCL_EXEC_CHECK_DISABLE
  _CCCL_TEMPLATE(class... _Args)
  _CCCL_REQUIRES(is_constructible_v<_Fn, _Args&&...>)
  _CCCL_API explicit constexpr __not_fn_t(_Args&&... __args) noexcept(is_nothrow_constructible_v<_Fn, _Args&&...>)
      : __f(::cuda::std::forward<_Args>(__args)...)
  {}

  _CCCL_EXEC_CHECK_DISABLE
  _CCCL_TEMPLATE(class... _Args)
  _CCCL_REQUIRES(__can_invoke_and_negate<_Fn&, _Args...>)
  _CCCL_API constexpr auto
  operator()(_Args&&... __args) & noexcept(noexcept(!::cuda::std::invoke(__f, ::cuda::std::forward<_Args>(__args)...)))
    -> decltype(!::cuda::std::invoke(__f, ::cuda::std::forward<_Args>(__args)...))
  {
    return !::cuda::std::invoke(__f, ::cuda::std::forward<_Args>(__args)...);
  }

  _CCCL_TEMPLATE(class... _Args)
  _CCCL_REQUIRES((!__can_invoke_and_negate<_Fn&, _Args...>) )
  void operator()(_Args&&...) & = delete;

  _CCCL_EXEC_CHECK_DISABLE
  _CCCL_TEMPLATE(class... _Args)
  _CCCL_REQUIRES(__can_invoke_and_negate<const _Fn&, _Args...>)
  _CCCL_API constexpr auto operator()(_Args&&... __args) const& noexcept(
    noexcept(!::cuda::std::invoke(__f, ::cuda::std::forward<_Args>(__args)...)))
    -> decltype(!::cuda::std::invoke(__f, ::cuda::std::forward<_Args>(__args)...))
  {
    return !::cuda::std::invoke(__f, ::cuda::std::forward<_Args>(__args)...);
  }

  _CCCL_TEMPLATE(class... _Args)
  _CCCL_REQUIRES((!__can_invoke_and_negate<const _Fn&, _Args...>) )
  void operator()(_Args&&...) const& = delete;

  _CCCL_EXEC_CHECK_DISABLE
  _CCCL_TEMPLATE(class... _Args)
  _CCCL_REQUIRES(__can_invoke_and_negate<_Fn, _Args...>)
  _CCCL_API constexpr auto operator()(_Args&&... __args) && noexcept(
    noexcept(!::cuda::std::invoke(::cuda::std::move(__f), ::cuda::std::forward<_Args>(__args)...)))
    -> decltype(!::cuda::std::invoke(::cuda::std::move(__f), ::cuda::std::forward<_Args>(__args)...))
  {
    return !::cuda::std::invoke(::cuda::std::move(__f), ::cuda::std::forward<_Args>(__args)...);
  }

  _CCCL_TEMPLATE(class... _Args)
  _CCCL_REQUIRES((!__can_invoke_and_negate<_Fn, _Args...>) )
  void operator()(_Args&&...) && = delete;

  _CCCL_EXEC_CHECK_DISABLE
  _CCCL_TEMPLATE(class... _Args)
  _CCCL_REQUIRES(__can_invoke_and_negate<const _Fn, _Args...>)
  _CCCL_API constexpr auto operator()(_Args&&... __args) const&& noexcept(
    noexcept(!::cuda::std::invoke(::cuda::std::move(__f), ::cuda::std::forward<_Args>(__args)...)))
    -> decltype(!::cuda::std::invoke(::cuda::std::move(__f), ::cuda::std::forward<_Args>(__args)...))
  {
    return !::cuda::std::invoke(::cuda::std::move(__f), ::cuda::std::forward<_Args>(__args)...);
  }

  _CCCL_TEMPLATE(class... _Args)
  _CCCL_REQUIRES((!__can_invoke_and_negate<const _Fn, _Args...>) )
  void operator()(_Args&&...) const&& = delete;
};

_CCCL_TEMPLATE(class _Fn)
_CCCL_REQUIRES(is_constructible_v<decay_t<_Fn>, _Fn> _CCCL_AND is_move_constructible_v<decay_t<_Fn>>)
[[nodiscard]] _CCCL_API constexpr auto not_fn(_Fn&& __f)
{
  return __not_fn_t<decay_t<_Fn>>(::cuda::std::forward<_Fn>(__f));
}

_CCCL_END_NAMESPACE_CUDA_STD

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA_STD___FUNCTIONAL_NOT_FN_H
