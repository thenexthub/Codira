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
#ifndef _CUDA_STD___RANGES_EMPTY_H
#define _CUDA_STD___RANGES_EMPTY_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/std/__concepts/class_or_enum.h>
#include <uscl/std/__iterator/concepts.h>
#include <uscl/std/__ranges/access.h>
#include <uscl/std/__ranges/size.h>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_RANGES

// [range.prim.empty]

_CCCL_BEGIN_NAMESPACE_CPO(__empty)

#if _CCCL_HAS_CONCEPTS()
template <class _Tp>
concept __member_empty = __workaround_52970<_Tp> && requires(_Tp&& __t) { bool(__t.empty()); };

template <class _Tp>
concept __can_invoke_size = !__member_empty<_Tp> && requires(_Tp&& __t) { ::cuda::std::ranges::size(__t); };

template <class _Tp>
concept __can_compare_begin_end = !__member_empty<_Tp> && !__can_invoke_size<_Tp> && requires(_Tp&& __t) {
  bool(::cuda::std::ranges::begin(__t) == ::cuda::std::ranges::end(__t));
  { ::cuda::std::ranges::begin(__t) } -> forward_iterator;
};
#else // ^^^ _CCCL_HAS_CONCEPTS() ^^^ / vvv !_CCCL_HAS_CONCEPTS() vvv
template <class _Tp>
_CCCL_CONCEPT_FRAGMENT(__member_empty_, requires(_Tp&& __t)(requires(__workaround_52970<_Tp>), (bool(__t.empty()))));

template <class _Tp>
_CCCL_CONCEPT __member_empty = _CCCL_FRAGMENT(__member_empty_, _Tp);

template <class _Tp>
_CCCL_CONCEPT_FRAGMENT(__can_invoke_size_,
                       requires(_Tp&& __t)(requires(!__member_empty<_Tp>), ((void) ::cuda::std::ranges::size(__t))));

template <class _Tp>
_CCCL_CONCEPT __can_invoke_size = _CCCL_FRAGMENT(__can_invoke_size_, _Tp);

template <class _Tp>
_CCCL_CONCEPT_FRAGMENT(
  __can_compare_begin_end_,
  requires(_Tp&& __t)(requires(!__member_empty<_Tp>),
                      requires(!__can_invoke_size<_Tp>),
                      (bool(::cuda::std::ranges::begin(__t) == ::cuda::std::ranges::end(__t))),
                      requires(forward_iterator<decltype(::cuda::std::ranges::begin(__t))>)));

template <class _Tp>
_CCCL_CONCEPT __can_compare_begin_end = _CCCL_FRAGMENT(__can_compare_begin_end_, _Tp);
#endif // ^^^ !_CCCL_HAS_CONCEPTS() ^^^

struct __fn
{
  _CCCL_TEMPLATE(class _Tp)
  _CCCL_REQUIRES(__member_empty<_Tp>)
  [[nodiscard]] _CCCL_API constexpr bool operator()(_Tp&& __t) const noexcept(noexcept(bool(__t.empty())))
  {
    return bool(__t.empty());
  }

  _CCCL_TEMPLATE(class _Tp)
  _CCCL_REQUIRES(__can_invoke_size<_Tp>)
  [[nodiscard]] _CCCL_API constexpr bool operator()(_Tp&& __t) const noexcept(noexcept(::cuda::std::ranges::size(__t)))
  {
    return ::cuda::std::ranges::size(__t) == 0;
  }

  _CCCL_TEMPLATE(class _Tp)
  _CCCL_REQUIRES(__can_compare_begin_end<_Tp>)
  [[nodiscard]] _CCCL_API constexpr bool operator()(_Tp&& __t) const
    noexcept(noexcept(bool(::cuda::std::ranges::begin(__t) == ::cuda::std::ranges::end(__t))))
  {
    return ::cuda::std::ranges::begin(__t) == ::cuda::std::ranges::end(__t);
  }
};
_CCCL_END_NAMESPACE_CPO

inline namespace __cpo
{
_CCCL_GLOBAL_CONSTANT auto empty = __empty::__fn{};
} // namespace __cpo

_CCCL_END_NAMESPACE_RANGES

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA_STD___RANGES_EMPTY_H
