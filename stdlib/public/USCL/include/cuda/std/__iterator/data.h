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

#ifndef _CUDA_STD___ITERATOR_DATA_H
#define _CUDA_STD___ITERATOR_DATA_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/std/cstddef>
#include <uscl/std/initializer_list>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA_STD

_CCCL_EXEC_CHECK_DISABLE
template <class _Cont>
[[nodiscard]] _CCCL_API constexpr auto data(_Cont& __c) noexcept(noexcept(__c.data())) -> decltype(__c.data())
{
  return __c.data();
}

_CCCL_EXEC_CHECK_DISABLE
template <class _Cont>
[[nodiscard]] _CCCL_API constexpr auto data(const _Cont& __c) noexcept(noexcept(__c.data())) -> decltype(__c.data())
{
  return __c.data();
}

template <class _Tp, size_t _Sz>
_CCCL_API constexpr _Tp* data(_Tp (&__array)[_Sz]) noexcept
{
  return __array;
}

template <class _Ep>
_CCCL_API constexpr const _Ep* data(initializer_list<_Ep> __il) noexcept
{
  return __il.begin();
}

_CCCL_END_NAMESPACE_CUDA_STD

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA_STD___ITERATOR_DATA_H
