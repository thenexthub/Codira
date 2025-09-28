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

#ifndef _CUDA_STD___ITERATOR_ACCESS_H
#define _CUDA_STD___ITERATOR_ACCESS_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/std/cstddef>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA_STD

namespace __begin
{
struct __fn
{
  template <class _Tp, size_t _Np>
  _CCCL_API constexpr _Tp* operator()(_Tp (&__array)[_Np]) const noexcept
  {
    return __array;
  }

  template <class _Cp>
  _CCCL_API constexpr auto operator()(_Cp& __c) const noexcept(noexcept(__c.begin())) -> decltype(__c.begin())
  {
    return __c.begin();
  }

  template <class _Cp>
  _CCCL_API constexpr auto operator()(const _Cp& __c) const noexcept(noexcept(__c.begin())) -> decltype(__c.begin())
  {
    return __c.begin();
  }
};
} // namespace __begin

inline namespace __cpo
{
_CCCL_GLOBAL_CONSTANT auto begin = __begin::__fn{};
} // namespace __cpo

namespace __end
{
struct __fn
{
  template <class _Tp, size_t _Np>
  _CCCL_API constexpr _Tp* operator()(_Tp (&__array)[_Np]) const noexcept
  {
    return __array + _Np;
  }

  template <class _Cp>
  _CCCL_API constexpr auto operator()(_Cp& __c) const noexcept(noexcept(__c.end())) -> decltype(__c.end())
  {
    return __c.end();
  }

  template <class _Cp>
  _CCCL_API constexpr auto operator()(const _Cp& __c) const noexcept(noexcept(__c.end())) -> decltype(__c.end())
  {
    return __c.end();
  }
};
} // namespace __end

inline namespace __cpo
{
_CCCL_GLOBAL_CONSTANT auto end = __end::__fn{};
} // namespace __cpo

namespace __cbegin
{
struct __fn
{
  template <class _Cp>
  _CCCL_API constexpr auto operator()(const _Cp& __c) const noexcept(noexcept(::cuda::std::begin(__c)))
    -> decltype(::cuda::std::begin(__c))
  {
    return ::cuda::std::begin(__c);
  }
};
} // namespace __cbegin

inline namespace __cpo
{
_CCCL_GLOBAL_CONSTANT auto cbegin = __cbegin::__fn{};
} // namespace __cpo

namespace __cend
{
struct __fn
{
  template <class _Cp>
  _CCCL_API constexpr auto operator()(const _Cp& __c) const noexcept(noexcept(::cuda::std::end(__c)))
    -> decltype(::cuda::std::end(__c))
  {
    return ::cuda::std::end(__c);
  }
};
} // namespace __cend

inline namespace __cpo
{
_CCCL_GLOBAL_CONSTANT auto cend = __cend::__fn{};
} // namespace __cpo

_CCCL_END_NAMESPACE_CUDA_STD

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA_STD___ITERATOR_ACCESS_H
