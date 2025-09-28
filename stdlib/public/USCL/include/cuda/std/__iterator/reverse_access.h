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

#ifndef _CUDA_STD___ITERATOR_REVERSE_ACCESS_H
#define _CUDA_STD___ITERATOR_REVERSE_ACCESS_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/std/__iterator/reverse_iterator.h>
#include <uscl/std/cstddef>
#include <uscl/std/initializer_list>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA_STD

namespace __rbegin
{
struct __fn
{
  template <class _Tp, size_t _Np>
  _CCCL_API constexpr reverse_iterator<_Tp*> operator()(_Tp (&__array)[_Np]) const noexcept
  {
    return reverse_iterator<_Tp*>(__array + _Np);
  }

  template <class _Ep>
  _CCCL_API constexpr reverse_iterator<const _Ep*> operator()(initializer_list<_Ep> __il) const noexcept
  {
    return reverse_iterator<const _Ep*>(__il.end());
  }

  template <class _Cp>
  _CCCL_API constexpr auto operator()(_Cp& __c) const noexcept(noexcept(__c.rbegin())) -> decltype(__c.rbegin())
  {
    return __c.rbegin();
  }

  template <class _Cp>
  _CCCL_API constexpr auto operator()(const _Cp& __c) const noexcept(noexcept(__c.rbegin())) -> decltype(__c.rbegin())
  {
    return __c.rbegin();
  }
};
} // namespace __rbegin

inline namespace __cpo
{
_CCCL_GLOBAL_CONSTANT auto rbegin = __rbegin::__fn{};
} // namespace __cpo

namespace __rend
{
struct __fn
{
  template <class _Tp, size_t _Np>
  _CCCL_API constexpr reverse_iterator<_Tp*> operator()(_Tp (&__array)[_Np]) const noexcept
  {
    return reverse_iterator<_Tp*>(__array);
  }

  template <class _Ep>
  _CCCL_API constexpr reverse_iterator<const _Ep*> operator()(initializer_list<_Ep> __il) const noexcept
  {
    return reverse_iterator<const _Ep*>(__il.begin());
  }

  template <class _Cp>
  _CCCL_API constexpr auto operator()(_Cp& __c) const noexcept(noexcept(__c.rend())) -> decltype(__c.rend())
  {
    return __c.rend();
  }

  template <class _Cp>
  _CCCL_API constexpr auto operator()(const _Cp& __c) const noexcept(noexcept(__c.rend())) -> decltype(__c.rend())
  {
    return __c.rend();
  }
};
} // namespace __rend

inline namespace __cpo
{
_CCCL_GLOBAL_CONSTANT auto rend = __rend::__fn{};
} // namespace __cpo

namespace __crbegin
{
struct __fn
{
  template <class _Cp>
  _CCCL_API constexpr auto operator()(const _Cp& __c) const noexcept(noexcept(::cuda::std::rbegin(__c)))
    -> decltype(::cuda::std::rbegin(__c))
  {
    return ::cuda::std::rbegin(__c);
  }
};
} // namespace __crbegin

inline namespace __cpo
{
_CCCL_GLOBAL_CONSTANT auto crbegin = __crbegin::__fn{};
} // namespace __cpo

namespace __crend
{
struct __fn
{
  template <class _Cp>
  _CCCL_API constexpr auto operator()(const _Cp& __c) const noexcept(noexcept(::cuda::std::rend(__c)))
    -> decltype(::cuda::std::rend(__c))
  {
    return ::cuda::std::rend(__c);
  }
};
} // namespace __crend

inline namespace __cpo
{
_CCCL_GLOBAL_CONSTANT auto crend = __crend::__fn{};
} // namespace __cpo

_CCCL_END_NAMESPACE_CUDA_STD

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA_STD___ITERATOR_REVERSE_ACCESS_H
