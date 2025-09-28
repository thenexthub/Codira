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
#ifndef _CUDA_STD___RANGES_DATA_H
#define _CUDA_STD___RANGES_DATA_H

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
#include <uscl/std/__iterator/reverse_iterator.h>
#include <uscl/std/__memory/pointer_traits.h>
#include <uscl/std/__ranges/access.h>
#include <uscl/std/__type_traits/is_object.h>
#include <uscl/std/__type_traits/is_pointer.h>
#include <uscl/std/__type_traits/is_reference.h>
#include <uscl/std/__type_traits/remove_pointer.h>
#include <uscl/std/__type_traits/remove_reference.h>
#include <uscl/std/__utility/auto_cast.h>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_RANGES

// [range.prim.data]

_CCCL_BEGIN_NAMESPACE_CPO(__data)

template <class _Tp>
_CCCL_CONCEPT __ptr_to_object = is_pointer_v<_Tp> && is_object_v<remove_pointer_t<_Tp>>;

#if _CCCL_HAS_CONCEPTS()
template <class _Tp>
concept __member_data = __can_borrow<_Tp> && __workaround_52970<_Tp> && requires(_Tp&& __t) {
  { _LIBCUDACXX_AUTO_CAST(__t.data()) } -> __ptr_to_object;
};

template <class _Tp>
concept __ranges_begin_invocable = !__member_data<_Tp> && __can_borrow<_Tp> && requires(_Tp&& __t) {
  { ::cuda::std::ranges::begin(__t) } -> contiguous_iterator;
};
#else // ^^^ _CCCL_HAS_CONCEPTS() ^^^ / vvv !_CCCL_HAS_CONCEPTS() vvv
template <class _Tp>
_CCCL_CONCEPT_FRAGMENT(__member_data_,
                       requires(_Tp&& __t)(requires(__can_borrow<_Tp>),
                                           requires(__workaround_52970<_Tp>),
                                           requires(__ptr_to_object<decltype(_LIBCUDACXX_AUTO_CAST(__t.data()))>)));

template <class _Tp>
_CCCL_CONCEPT __member_data = _CCCL_FRAGMENT(__member_data_, _Tp);

template <class _Tp>
_CCCL_CONCEPT_FRAGMENT(__ranges_begin_invocable_,
                       requires(_Tp&& __t)(requires(!__member_data<_Tp>),
                                           requires(__can_borrow<_Tp>),
                                           requires(contiguous_iterator<decltype(::cuda::std::ranges::begin(__t))>)));

template <class _Tp>
_CCCL_CONCEPT __ranges_begin_invocable = _CCCL_FRAGMENT(__ranges_begin_invocable_, _Tp);
#endif // ^^^ !_CCCL_HAS_CONCEPTS() ^^^

struct __fn
{
  _CCCL_EXEC_CHECK_DISABLE
  _CCCL_TEMPLATE(class _Tp)
  _CCCL_REQUIRES(__member_data<_Tp>)
  [[nodiscard]] _CCCL_API constexpr auto operator()(_Tp&& __t) const noexcept(noexcept(__t.data()))
  {
    return __t.data();
  }

  _CCCL_EXEC_CHECK_DISABLE
  _CCCL_TEMPLATE(class _Tp)
  _CCCL_REQUIRES(__ranges_begin_invocable<_Tp>)
  [[nodiscard]] _CCCL_API constexpr auto operator()(_Tp&& __t) const
    noexcept(noexcept(::cuda::std::to_address(::cuda::std::ranges::begin(__t))))
  {
    return ::cuda::std::to_address(::cuda::std::ranges::begin(__t));
  }
};
_CCCL_END_NAMESPACE_CPO

inline namespace __cpo
{
_CCCL_GLOBAL_CONSTANT auto data = __data::__fn{};
} // namespace __cpo

// [range.prim.cdata]

_CCCL_BEGIN_NAMESPACE_CPO(__cdata)
struct __fn
{
  _CCCL_TEMPLATE(class _Tp)
  _CCCL_REQUIRES(is_lvalue_reference_v<_Tp&&>)
  [[nodiscard]] _CCCL_API constexpr auto operator()(_Tp&& __t) const
    noexcept(noexcept(::cuda::std::ranges::data(static_cast<const remove_reference_t<_Tp>&>(__t))))
      -> decltype(::cuda::std::ranges::data(static_cast<const remove_reference_t<_Tp>&>(__t)))
  {
    return ::cuda::std::ranges::data(static_cast<const remove_reference_t<_Tp>&>(__t));
  }

  _CCCL_TEMPLATE(class _Tp)
  _CCCL_REQUIRES(is_rvalue_reference_v<_Tp&&>)
  [[nodiscard]] _CCCL_API constexpr auto operator()(_Tp&& __t) const
    noexcept(noexcept(::cuda::std::ranges::data(static_cast<const _Tp&&>(__t))))
      -> decltype(::cuda::std::ranges::data(static_cast<const _Tp&&>(__t)))
  {
    return ::cuda::std::ranges::data(static_cast<const _Tp&&>(__t));
  }
};
_CCCL_END_NAMESPACE_CPO

inline namespace __cpo
{
_CCCL_GLOBAL_CONSTANT auto cdata = __cdata::__fn{};
} // namespace __cpo

_CCCL_END_NAMESPACE_RANGES

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA_STD___RANGES_DATA_H
