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

#ifndef _CUDA_STD___NUMERIC_TRANSFORM_REDUCE_H
#define _CUDA_STD___NUMERIC_TRANSFORM_REDUCE_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/std/__functional/operations.h>
#include <uscl/std/__iterator/iterator_traits.h>
#include <uscl/std/__utility/move.h>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA_STD

template <class _InputIterator, class _Tp, class _BinaryOp, class _UnaryOp>
[[nodiscard]] _CCCL_API constexpr _Tp
transform_reduce(_InputIterator __first, _InputIterator __last, _Tp __init, _BinaryOp __b, _UnaryOp __u)
{
  for (; __first != __last; ++__first)
  {
    __init = __b(::cuda::std::move(__init), __u(*__first));
  }
  return __init;
}

template <class _InputIterator1, class _InputIterator2, class _Tp, class _BinaryOp1, class _BinaryOp2>
[[nodiscard]] _CCCL_API constexpr _Tp transform_reduce(
  _InputIterator1 __first1,
  _InputIterator1 __last1,
  _InputIterator2 __first2,
  _Tp __init,
  _BinaryOp1 __b1,
  _BinaryOp2 __b2)
{
  for (; __first1 != __last1; ++__first1, (void) ++__first2)
  {
    __init = __b1(::cuda::std::move(__init), __b2(*__first1, *__first2));
  }
  return __init;
}

template <class _InputIterator1, class _InputIterator2, class _Tp>
[[nodiscard]] _CCCL_API constexpr _Tp
transform_reduce(_InputIterator1 __first1, _InputIterator1 __last1, _InputIterator2 __first2, _Tp __init)
{
  return ::cuda::std::transform_reduce(
    __first1, __last1, __first2, ::cuda::std::move(__init), ::cuda::std::plus<>(), ::cuda::std::multiplies<>());
}

_CCCL_END_NAMESPACE_CUDA_STD

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA_STD___NUMERIC_TRANSFORM_REDUCE_H
