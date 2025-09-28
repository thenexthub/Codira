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
#ifndef _CUDA_STD___RANGES_REND_H
#define _CUDA_STD___RANGES_REND_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/std/__concepts/class_or_enum.h>
#include <uscl/std/__concepts/same_as.h>
#include <uscl/std/__iterator/concepts.h>
#include <uscl/std/__iterator/readable_traits.h>
#include <uscl/std/__iterator/reverse_iterator.h>
#include <uscl/std/__ranges/access.h>
#include <uscl/std/__ranges/rbegin.h>
#include <uscl/std/__type_traits/is_reference.h>
#include <uscl/std/__type_traits/remove_cvref.h>
#include <uscl/std/__type_traits/remove_reference.h>
#include <uscl/std/__utility/auto_cast.h>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_RANGES

// [range.access.rend]

_CCCL_BEGIN_NAMESPACE_CPO(__rend)
template <class _Tp>
void rend(_Tp&) = delete;
template <class _Tp>
void rend(const _Tp&) = delete;

#if _CCCL_HAS_CONCEPTS()
template <class _Tp>
concept __member_rend = __can_borrow<_Tp> && __workaround_52970<_Tp> && requires(_Tp&& __t) {
  ::cuda::std::ranges::rbegin(__t);
  { _LIBCUDACXX_AUTO_CAST(__t.rend()) } -> sentinel_for<decltype(::cuda::std::ranges::rbegin(__t))>;
};

template <class _Tp>
concept __unqualified_rend =
  !__member_rend<_Tp> && __can_borrow<_Tp> && __class_or_enum<remove_cvref_t<_Tp>> && requires(_Tp&& __t) {
    ::cuda::std::ranges::rbegin(__t);
    { _LIBCUDACXX_AUTO_CAST(rend(__t)) } -> sentinel_for<decltype(::cuda::std::ranges::rbegin(__t))>;
  };

template <class _Tp>
concept __can_reverse = __can_borrow<_Tp> && !__member_rend<_Tp> && !__unqualified_rend<_Tp> && requires(_Tp&& __t) {
  { ::cuda::std::ranges::begin(__t) } -> same_as<decltype(::cuda::std::ranges::end(__t))>;
  { ::cuda::std::ranges::begin(__t) } -> bidirectional_iterator;
};
#else // ^^^ _CCCL_HAS_CONCEPTS() ^^^ / vvv !_CCCL_HAS_CONCEPTS() vvv
template <class _Tp>
_CCCL_CONCEPT_FRAGMENT(
  __member_rend_,
  requires(_Tp&& __t)(
    requires(__can_borrow<_Tp>),
    requires(__workaround_52970<_Tp>),
    typename(decltype(::cuda::std::ranges::rbegin(__t))),
    requires(sentinel_for<decltype(_LIBCUDACXX_AUTO_CAST(__t.rend())), decltype(::cuda::std::ranges::rbegin(__t))>)));

template <class _Tp>
_CCCL_CONCEPT __member_rend = _CCCL_FRAGMENT(__member_rend_, _Tp);

template <class _Tp>
_CCCL_CONCEPT_FRAGMENT(
  __unqualified_rend_,
  requires(_Tp&& __t)(
    requires(!__member_rend<_Tp>),
    requires(__can_borrow<_Tp>),
    requires(__class_or_enum<remove_cvref_t<_Tp>>),
    typename(decltype(::cuda::std::ranges::rbegin(__t))),
    requires(sentinel_for<decltype(_LIBCUDACXX_AUTO_CAST(rend(__t))), decltype(::cuda::std::ranges::rbegin(__t))>)));

template <class _Tp>
_CCCL_CONCEPT __unqualified_rend = _CCCL_FRAGMENT(__unqualified_rend_, _Tp);

template <class _Tp>
_CCCL_CONCEPT_FRAGMENT(
  __can_reverse_,
  requires(_Tp&& __t)(
    requires(!__member_rend<_Tp>),
    requires(!__unqualified_rend<_Tp>),
    requires(__can_borrow<_Tp>),
    requires(same_as<decltype(::cuda::std::ranges::begin(__t)), decltype(::cuda::std::ranges::end(__t))>),
    requires(bidirectional_iterator<decltype(::cuda::std::ranges::begin(__t))>)));

template <class _Tp>
_CCCL_CONCEPT __can_reverse = _CCCL_FRAGMENT(__can_reverse_, _Tp);
#endif // ^^^ !_CCCL_HAS_CONCEPTS() ^^^

class __fn
{
public:
  _CCCL_EXEC_CHECK_DISABLE
  _CCCL_TEMPLATE(class _Tp)
  _CCCL_REQUIRES(__member_rend<_Tp>)
  [[nodiscard]] _CCCL_API constexpr auto operator()(_Tp&& __t) const
    noexcept(noexcept(_LIBCUDACXX_AUTO_CAST(__t.rend())))
  {
    return _LIBCUDACXX_AUTO_CAST(__t.rend());
  }

  _CCCL_EXEC_CHECK_DISABLE
  _CCCL_TEMPLATE(class _Tp)
  _CCCL_REQUIRES(__unqualified_rend<_Tp>)
  [[nodiscard]] _CCCL_API constexpr auto operator()(_Tp&& __t) const
    noexcept(noexcept(_LIBCUDACXX_AUTO_CAST(rend(__t))))
  {
    return _LIBCUDACXX_AUTO_CAST(rend(__t));
  }

  _CCCL_EXEC_CHECK_DISABLE
  _CCCL_TEMPLATE(class _Tp)
  _CCCL_REQUIRES(__can_reverse<_Tp>)
  [[nodiscard]] _CCCL_API constexpr auto operator()(_Tp&& __t) const noexcept(noexcept(::cuda::std::ranges::begin(__t)))
  {
    return ::cuda::std::make_reverse_iterator(::cuda::std::ranges::begin(__t));
  }

  _CCCL_TEMPLATE(class _Tp)
  _CCCL_REQUIRES((!__member_rend<_Tp> && !__unqualified_rend<_Tp> && !__can_reverse<_Tp>) )
  void operator()(_Tp&&) const = delete;
};
_CCCL_END_NAMESPACE_CPO

inline namespace __cpo
{
_CCCL_GLOBAL_CONSTANT auto rend = __rend::__fn{};
} // namespace __cpo

// [range.access.crend]

_CCCL_BEGIN_NAMESPACE_CPO(__crend)
struct __fn
{
  _CCCL_EXEC_CHECK_DISABLE
  _CCCL_TEMPLATE(class _Tp)
  _CCCL_REQUIRES(is_lvalue_reference_v<_Tp&&>)
  [[nodiscard]] _CCCL_API constexpr auto operator()(_Tp&& __t) const
    noexcept(noexcept(::cuda::std::ranges::rend(static_cast<const remove_reference_t<_Tp>&>(__t))))
      -> decltype(::cuda::std::ranges::rend(static_cast<const remove_reference_t<_Tp>&>(__t)))
  {
    return ::cuda::std::ranges::rend(static_cast<const remove_reference_t<_Tp>&>(__t));
  }

  _CCCL_EXEC_CHECK_DISABLE
  _CCCL_TEMPLATE(class _Tp)
  _CCCL_REQUIRES(is_rvalue_reference_v<_Tp&&>)
  [[nodiscard]] _CCCL_API constexpr auto operator()(_Tp&& __t) const
    noexcept(noexcept(::cuda::std::ranges::rend(static_cast<const _Tp&&>(__t))))
      -> decltype(::cuda::std::ranges::rend(static_cast<const _Tp&&>(__t)))
  {
    return ::cuda::std::ranges::rend(static_cast<const _Tp&&>(__t));
  }
};
_CCCL_END_NAMESPACE_CPO

inline namespace __cpo
{
_CCCL_GLOBAL_CONSTANT auto crend = __crend::__fn{};
} // namespace __cpo

_CCCL_END_NAMESPACE_RANGES

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA_STD___RANGES_REND_H
