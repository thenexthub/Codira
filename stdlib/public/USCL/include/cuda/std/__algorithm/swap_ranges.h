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

#ifndef _CUDA_STD___ALGORITHM_SWAP_RANGES_H
#define _CUDA_STD___ALGORITHM_SWAP_RANGES_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/std/__algorithm/iterator_operations.h>
#include <uscl/std/__utility/move.h>
#include <uscl/std/__utility/pair.h>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA_STD

// 2+2 iterators: the shorter size will be used.
_CCCL_EXEC_CHECK_DISABLE
template <class _AlgPolicy, class _ForwardIterator1, class _Sentinel1, class _ForwardIterator2, class _Sentinel2>
_CCCL_API constexpr pair<_ForwardIterator1, _ForwardIterator2>
__swap_ranges(_ForwardIterator1 __first1, _Sentinel1 __last1, _ForwardIterator2 __first2, _Sentinel2 __last2)
{
  while (__first1 != __last1 && __first2 != __last2)
  {
    _IterOps<_AlgPolicy>::iter_swap(__first1, __first2);
    ++__first1;
    ++__first2;
  }

  return pair<_ForwardIterator1, _ForwardIterator2>(::cuda::std::move(__first1), ::cuda::std::move(__first2));
}

// 2+1 iterators: size2 >= size1.
_CCCL_EXEC_CHECK_DISABLE
template <class _AlgPolicy, class _ForwardIterator1, class _Sentinel1, class _ForwardIterator2>
_CCCL_API constexpr pair<_ForwardIterator1, _ForwardIterator2>
__swap_ranges(_ForwardIterator1 __first1, _Sentinel1 __last1, _ForwardIterator2 __first2)
{
  while (__first1 != __last1)
  {
    _IterOps<_AlgPolicy>::iter_swap(__first1, __first2);
    ++__first1;
    ++__first2;
  }

  return pair<_ForwardIterator1, _ForwardIterator2>(::cuda::std::move(__first1), ::cuda::std::move(__first2));
}

_CCCL_EXEC_CHECK_DISABLE
template <class _ForwardIterator1, class _ForwardIterator2>
_CCCL_API constexpr _ForwardIterator2
swap_ranges(_ForwardIterator1 __first1, _ForwardIterator1 __last1, _ForwardIterator2 __first2)
{
  return ::cuda::std::__swap_ranges<_ClassicAlgPolicy>(
           ::cuda::std::move(__first1), ::cuda::std::move(__last1), ::cuda::std::move(__first2))
    .second;
}

_CCCL_END_NAMESPACE_CUDA_STD

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA_STD___ALGORITHM_SWAP_RANGES_H
