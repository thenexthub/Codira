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

#ifndef _CUDA_STD___ALGORITHM_UNWRAP_RANGE_H
#define _CUDA_STD___ALGORITHM_UNWRAP_RANGE_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/std/__algorithm/unwrap_iter.h>
#include <uscl/std/__concepts/constructible.h>
#include <uscl/std/__iterator/concepts.h>
#include <uscl/std/__iterator/next.h>
#include <uscl/std/__utility/declval.h>
#include <uscl/std/__utility/move.h>
#include <uscl/std/__utility/pair.h>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA_STD

// __unwrap_range and __rewrap_range are used to unwrap ranges which may have different iterator and sentinel types.
// __unwrap_iter and __rewrap_iter don't work for this, because they assume that the iterator and sentinel have
// the same type. __unwrap_range tries to get two iterators and then forward to __unwrap_iter.

#if _CCCL_STD_VER >= 2020
template <class _Iter, class _Sent>
struct __unwrap_range_impl
{
  _CCCL_EXEC_CHECK_DISABLE
  _CCCL_API static constexpr auto __unwrap(_Iter __first, _Sent __sent)
    requires random_access_iterator<_Iter> && sized_sentinel_for<_Sent, _Iter>
  {
    auto __last = ranges::next(__first, __sent);
    return pair{::cuda::std::__unwrap_iter(::cuda::std::move(__first)),
                ::cuda::std::__unwrap_iter(::cuda::std::move(__last))};
  }

  _CCCL_EXEC_CHECK_DISABLE
  _CCCL_API static constexpr auto __unwrap(_Iter __first, _Sent __last)
  {
    return pair{::cuda::std::move(__first), ::cuda::std::move(__last)};
  }

  _CCCL_EXEC_CHECK_DISABLE
  _CCCL_API static constexpr auto
  __rewrap(_Iter __orig_iter, decltype(::cuda::std::__unwrap_iter(::cuda::std::move(__orig_iter))) __iter)
    requires random_access_iterator<_Iter> && sized_sentinel_for<_Sent, _Iter>
  {
    return ::cuda::std::__rewrap_iter(::cuda::std::move(__orig_iter), ::cuda::std::move(__iter));
  }

  _CCCL_EXEC_CHECK_DISABLE
  _CCCL_API static constexpr auto __rewrap(const _Iter&, _Iter __iter)
    requires(!(random_access_iterator<_Iter> && sized_sentinel_for<_Sent, _Iter>) )
  {
    return __iter;
  }
};

template <class _Iter>
struct __unwrap_range_impl<_Iter, _Iter>
{
  _CCCL_EXEC_CHECK_DISABLE
  _CCCL_API static constexpr auto __unwrap(_Iter __first, _Iter __last)
  {
    return pair{::cuda::std::__unwrap_iter(::cuda::std::move(__first)),
                ::cuda::std::__unwrap_iter(::cuda::std::move(__last))};
  }

  _CCCL_EXEC_CHECK_DISABLE
  _CCCL_API static constexpr auto __rewrap(_Iter __orig_iter, decltype(::cuda::std::__unwrap_iter(__orig_iter)) __iter)
  {
    return ::cuda::std::__rewrap_iter(::cuda::std::move(__orig_iter), ::cuda::std::move(__iter));
  }
};

_CCCL_EXEC_CHECK_DISABLE
template <class _Iter, class _Sent>
_CCCL_API constexpr auto __unwrap_range(_Iter __first, _Sent __last)
{
  return __unwrap_range_impl<_Iter, _Sent>::__unwrap(::cuda::std::move(__first), ::cuda::std::move(__last));
}

_CCCL_EXEC_CHECK_DISABLE
template <class _Sent, class _Iter, class _Unwrapped>
_CCCL_API constexpr _Iter __rewrap_range(_Iter __orig_iter, _Unwrapped __iter)
{
  return __unwrap_range_impl<_Iter, _Sent>::__rewrap(::cuda::std::move(__orig_iter), ::cuda::std::move(__iter));
}
#else // ^^^ C++20 ^^^ / vvv C++17 vvv
_CCCL_EXEC_CHECK_DISABLE
template <class _Iter, class _Unwrapped = decltype(::cuda::std::__unwrap_iter(::cuda::std::declval<_Iter>()))>
_CCCL_API constexpr pair<_Unwrapped, _Unwrapped> __unwrap_range(_Iter __first, _Iter __last)
{
  return ::cuda::std::make_pair(
    ::cuda::std::__unwrap_iter(::cuda::std::move(__first)), ::cuda::std::__unwrap_iter(::cuda::std::move(__last)));
}

_CCCL_EXEC_CHECK_DISABLE
template <class _Iter, class _Unwrapped>
_CCCL_API constexpr _Iter __rewrap_range(_Iter __orig_iter, _Unwrapped __iter)
{
  return ::cuda::std::__rewrap_iter(::cuda::std::move(__orig_iter), ::cuda::std::move(__iter));
}
#endif // _CCCL_STD_VER <= 2017

_CCCL_END_NAMESPACE_CUDA_STD

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA_STD___ALGORITHM_UNWRAP_RANGE_H
