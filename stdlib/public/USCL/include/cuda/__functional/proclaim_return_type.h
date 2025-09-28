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

#ifndef _CUDA___FUNCTIONAL_PROCLAIM_RETURN_TYPE_H
#define _CUDA___FUNCTIONAL_PROCLAIM_RETURN_TYPE_H

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
#include <uscl/std/__type_traits/is_same.h>
#include <uscl/std/__utility/forward.h>
#include <uscl/std/__utility/move.h>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA
namespace __detail
{

template <class _Ret, class _DecayFn>
class __return_type_wrapper
{
private:
  _DecayFn __fn_;

public:
  __return_type_wrapper() = delete;

  template <class _Fn,
            class = ::cuda::std::enable_if_t<::cuda::std::is_same<::cuda::std::decay_t<_Fn>, _DecayFn>::value>>
  _CCCL_API constexpr explicit __return_type_wrapper(_Fn&& __fn) noexcept
      : __fn_(::cuda::std::forward<_Fn>(__fn))
  {}

  template <class... _As>
  _CCCL_API constexpr _Ret operator()(_As&&... __as) & noexcept
  {
#if !_CCCL_CUDA_COMPILER(NVCC) || defined(__CUDA_ARCH__)
    static_assert(::cuda::std::is_same<_Ret, typename ::cuda::std::__invoke_of<_DecayFn&, _As...>::type>::value,
                  "Return type shall match the proclaimed one exactly");
#endif // !_CCCL_CUDA_COMPILER(NVCC) || __CUDA_ARCH__

    return ::cuda::std::__invoke(__fn_, ::cuda::std::forward<_As>(__as)...);
  }

  template <class... _As>
  _CCCL_API constexpr _Ret operator()(_As&&... __as) && noexcept
  {
#if !_CCCL_CUDA_COMPILER(NVCC) || defined(__CUDA_ARCH__)
    static_assert(::cuda::std::is_same<_Ret, typename ::cuda::std::__invoke_of<_DecayFn, _As...>::type>::value,
                  "Return type shall match the proclaimed one exactly");
#endif // !_CCCL_CUDA_COMPILER(NVCC) || __CUDA_ARCH__

    return ::cuda::std::__invoke(::cuda::std::move(__fn_), ::cuda::std::forward<_As>(__as)...);
  }

  template <class... _As>
  _CCCL_API constexpr _Ret operator()(_As&&... __as) const& noexcept
  {
#if !_CCCL_CUDA_COMPILER(NVCC) || defined(__CUDA_ARCH__)
    static_assert(::cuda::std::is_same<_Ret, typename ::cuda::std::__invoke_of<const _DecayFn&, _As...>::type>::value,
                  "Return type shall match the proclaimed one exactly");
#endif // !_CCCL_CUDA_COMPILER(NVCC) || __CUDA_ARCH__

    return ::cuda::std::__invoke(__fn_, ::cuda::std::forward<_As>(__as)...);
  }

  template <class... _As>
  _CCCL_API constexpr _Ret operator()(_As&&... __as) const&& noexcept
  {
#if !_CCCL_CUDA_COMPILER(NVCC) || defined(__CUDA_ARCH__)
    static_assert(::cuda::std::is_same<_Ret, typename ::cuda::std::__invoke_of<const _DecayFn, _As...>::type>::value,
                  "Return type shall match the proclaimed one exactly");
#endif // !_CCCL_CUDA_COMPILER(NVCC) || __CUDA_ARCH__

    return ::cuda::std::__invoke(::cuda::std::move(__fn_), ::cuda::std::forward<_As>(__as)...);
  }
};

} // namespace __detail

template <class _Ret, class _Fn>
_CCCL_API inline __detail::__return_type_wrapper<_Ret, ::cuda::std::decay_t<_Fn>>
proclaim_return_type(_Fn&& __fn) noexcept
{
  return __detail::__return_type_wrapper<_Ret, ::cuda::std::decay_t<_Fn>>(::cuda::std::forward<_Fn>(__fn));
}
_CCCL_END_NAMESPACE_CUDA

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA___FUNCTIONAL_PROCLAIM_RETURN_TYPE_H
