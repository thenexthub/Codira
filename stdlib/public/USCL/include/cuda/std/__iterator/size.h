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

#ifndef _CUDA_STD___ITERATOR_SIZE_H
#define _CUDA_STD___ITERATOR_SIZE_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/std/__type_traits/common_type.h>
#include <uscl/std/__type_traits/make_signed.h>
#include <uscl/std/cstddef>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA_STD

_CCCL_EXEC_CHECK_DISABLE
template <class _Cont>
_CCCL_API constexpr auto size(const _Cont& __c) noexcept(noexcept(__c.size())) -> decltype(__c.size())
{
  return __c.size();
}

template <class _Tp, size_t _Sz>
_CCCL_API constexpr size_t size(const _Tp (&)[_Sz]) noexcept
{
  return _Sz;
}

_CCCL_EXEC_CHECK_DISABLE
template <class _Cont>
_CCCL_API constexpr auto ssize(const _Cont& __c) noexcept(
  noexcept(static_cast<common_type_t<ptrdiff_t, make_signed_t<decltype(__c.size())>>>(__c.size())))
  -> common_type_t<ptrdiff_t, make_signed_t<decltype(__c.size())>>
{
  return static_cast<common_type_t<ptrdiff_t, make_signed_t<decltype(__c.size())>>>(__c.size());
}

// GCC complains about the implicit conversion from ptrdiff_t to size_t in
// the array bound.
_CCCL_DIAG_PUSH
_CCCL_DIAG_SUPPRESS_GCC("-Wsign-conversion")
template <class _Tp, ptrdiff_t _Sz>
_CCCL_API constexpr ptrdiff_t ssize(const _Tp (&)[_Sz]) noexcept
{
  return _Sz;
}
_CCCL_DIAG_POP

_CCCL_END_NAMESPACE_CUDA_STD

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA_STD___ITERATOR_SIZE_H
