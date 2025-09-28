/*
 *
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 * Date: Sunday, April 14, 2024.
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

#ifndef _CUDA_STD___MDSPAN_EMPTY_BASE_H
#define _CUDA_STD___MDSPAN_EMPTY_BASE_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/std/__type_traits/enable_if.h>
#include <uscl/std/__type_traits/is_constructible.h>
#include <uscl/std/__type_traits/is_default_constructible.h>
#include <uscl/std/__type_traits/is_empty.h>
#include <uscl/std/__type_traits/is_nothrow_constructible.h>
#include <uscl/std/__type_traits/is_nothrow_default_constructible.h>
#include <uscl/std/__utility/forward.h>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA_STD

template <size_t _Index, class _Elem, bool = is_empty_v<_Elem>>
struct _CCCL_DECLSPEC_EMPTY_BASES __mdspan_ebco_impl
{
  _Elem __elem_;

  _CCCL_EXEC_CHECK_DISABLE
  _CCCL_TEMPLATE(class _Elem_ = _Elem)
  _CCCL_REQUIRES(is_default_constructible_v<_Elem_>)
  _CCCL_API constexpr __mdspan_ebco_impl() noexcept(is_nothrow_default_constructible_v<_Elem_>)
      : __elem_()
  {}

  _CCCL_EXEC_CHECK_DISABLE
  _CCCL_TEMPLATE(class... _Args)
  _CCCL_REQUIRES((sizeof...(_Args) != 0) _CCCL_AND is_constructible_v<_Elem, _Args...>)
  _CCCL_API constexpr __mdspan_ebco_impl(_Args&&... __args) noexcept(is_nothrow_constructible_v<_Elem, _Args...>)
      : __elem_(::cuda::std::forward<_Args>(__args)...)
  {}

  [[nodiscard]] _CCCL_API constexpr _Elem& __get() noexcept
  {
    return __elem_;
  }
  [[nodiscard]] _CCCL_API constexpr const _Elem& __get() const noexcept
  {
    return __elem_;
  }
};

template <size_t _Index, class _Elem>
struct _CCCL_DECLSPEC_EMPTY_BASES __mdspan_ebco_impl<_Index, _Elem, true> : _Elem
{
  _CCCL_EXEC_CHECK_DISABLE
  _CCCL_TEMPLATE(class _Elem_ = _Elem)
  _CCCL_REQUIRES(is_default_constructible_v<_Elem_>)
  _CCCL_API constexpr __mdspan_ebco_impl() noexcept(is_nothrow_default_constructible_v<_Elem_>)
      : _Elem()
  {}

  _CCCL_EXEC_CHECK_DISABLE
  _CCCL_TEMPLATE(class... _Args)
  _CCCL_REQUIRES((sizeof...(_Args) != 0) _CCCL_AND is_constructible_v<_Elem, _Args...>)
  _CCCL_API constexpr __mdspan_ebco_impl(_Args&&... __args) noexcept(is_nothrow_constructible_v<_Elem, _Args...>)
      : _Elem(::cuda::std::forward<_Args>(__args)...)
  {}

  [[nodiscard]] _CCCL_API constexpr _Elem& __get() noexcept
  {
    return *static_cast<_Elem*>(this);
  }
  [[nodiscard]] _CCCL_API constexpr const _Elem& __get() const noexcept
  {
    return *static_cast<const _Elem*>(this);
  }
};

template <class...>
struct _CCCL_DECLSPEC_EMPTY_BASES __mdspan_ebco;

template <class _Elem1>
struct _CCCL_DECLSPEC_EMPTY_BASES __mdspan_ebco<_Elem1> : __mdspan_ebco_impl<0, _Elem1>
{
  using __base1 = __mdspan_ebco_impl<0, _Elem1>;

  _CCCL_EXEC_CHECK_DISABLE
  _CCCL_TEMPLATE(class _Elem1_ = _Elem1)
  _CCCL_REQUIRES(is_default_constructible_v<_Elem1_>)
  _CCCL_API constexpr __mdspan_ebco() noexcept(is_nothrow_default_constructible_v<_Elem1_>)
      : __base1()
  {}

  _CCCL_EXEC_CHECK_DISABLE
  _CCCL_TEMPLATE(class... _Args)
  _CCCL_REQUIRES((sizeof...(_Args) != 0) _CCCL_AND is_constructible_v<_Elem1, _Args...>)
  _CCCL_API constexpr __mdspan_ebco(_Args&&... __args) noexcept(is_nothrow_constructible_v<_Elem1, _Args...>)
      : __base1(::cuda::std::forward<_Args>(__args)...)
  {}

  _CCCL_TEMPLATE(size_t _Index)
  _CCCL_REQUIRES((_Index < 1))
  [[nodiscard]] _CCCL_API constexpr _Elem1& __get() noexcept
  {
    return static_cast<__base1*>(this)->__get();
  }

  _CCCL_TEMPLATE(size_t _Index)
  _CCCL_REQUIRES((_Index < 1))
  [[nodiscard]] _CCCL_API constexpr const _Elem1& __get() const noexcept
  {
    return static_cast<const __base1*>(this)->__get();
  }

  _CCCL_EXEC_CHECK_DISABLE
  _CCCL_API friend constexpr void swap(__mdspan_ebco& __x, __mdspan_ebco& __y)
  {
    swap(__x.__get<0>(), __y.__get<0>());
  }
};

template <class _Elem1, class _Elem2>
struct _CCCL_DECLSPEC_EMPTY_BASES __mdspan_ebco<_Elem1, _Elem2>
    : __mdspan_ebco_impl<0, _Elem1>
    , __mdspan_ebco_impl<1, _Elem2>
{
  using __base1 = __mdspan_ebco_impl<0, _Elem1>;
  using __base2 = __mdspan_ebco_impl<1, _Elem2>;

  _CCCL_EXEC_CHECK_DISABLE
  _CCCL_TEMPLATE(class _Elem1_ = _Elem1, class _Elem2_ = _Elem2)
  _CCCL_REQUIRES(is_default_constructible_v<_Elem1_> _CCCL_AND is_default_constructible_v<_Elem2_>)
  _CCCL_API constexpr __mdspan_ebco() noexcept(is_nothrow_default_constructible_v<_Elem1_>
                                               && is_nothrow_default_constructible_v<_Elem2_>)
      : __base1()
      , __base2()
  {}

  template <class _Arg1>
  static constexpr bool __is_constructible_from_one_arg =
    is_constructible_v<_Elem1, _Arg1> && is_default_constructible_v<_Elem2>;

  template <class _Arg1>
  static constexpr bool __is_nothrow_constructible_from_one_arg =
    is_nothrow_constructible_v<_Elem1, _Arg1> && is_nothrow_default_constructible_v<_Elem2>;

  _CCCL_EXEC_CHECK_DISABLE
  _CCCL_TEMPLATE(class _Arg1)
  _CCCL_REQUIRES(__is_constructible_from_one_arg<_Arg1>)
  _CCCL_API constexpr __mdspan_ebco(_Arg1&& __arg1) noexcept(__is_nothrow_constructible_from_one_arg<_Arg1>)
      : __base1(::cuda::std::forward<_Arg1>(__arg1))
      , __base2()
  {}

  template <class _Arg1, class _Arg2>
  static constexpr bool __is_constructible_from_two_args =
    is_constructible_v<_Elem1, _Arg1> && is_constructible_v<_Elem2, _Arg2>;

  template <class _Arg1, class _Arg2>
  static constexpr bool __is_nothrow_constructible_from_two_args =
    is_nothrow_constructible_v<_Elem1, _Arg1> && is_nothrow_constructible_v<_Elem2, _Arg2>;

  _CCCL_EXEC_CHECK_DISABLE
  _CCCL_TEMPLATE(class _Arg1, class _Arg2)
  _CCCL_REQUIRES(__is_constructible_from_two_args<_Arg1, _Arg2>)
  _CCCL_API constexpr __mdspan_ebco(_Arg1&& __arg1,
                                    _Arg2&& __arg2) noexcept(__is_nothrow_constructible_from_two_args<_Arg1, _Arg2>)
      : __base1(::cuda::std::forward<_Arg1>(__arg1))
      , __base2(::cuda::std::forward<_Arg2>(__arg2))
  {}

  _CCCL_TEMPLATE(size_t _Index)
  _CCCL_REQUIRES((_Index < 2))
  [[nodiscard]] _CCCL_API constexpr decltype(auto) __get() noexcept
  {
    if constexpr (_Index == 0)
    {
      return static_cast<__base1*>(this)->__get();
    }
    else // if constexpr (_Index == 1)
    {
      return static_cast<__base2*>(this)->__get();
    }
    _CCCL_UNREACHABLE();
  }

  _CCCL_TEMPLATE(size_t _Index)
  _CCCL_REQUIRES((_Index < 2))
  [[nodiscard]] _CCCL_API constexpr decltype(auto) __get() const noexcept
  {
    if constexpr (_Index == 0)
    {
      return static_cast<const __base1*>(this)->__get();
    }
    else // if constexpr (_Index == 1)
    {
      return static_cast<const __base2*>(this)->__get();
    }
    _CCCL_UNREACHABLE();
  }

  _CCCL_EXEC_CHECK_DISABLE
  _CCCL_API friend constexpr void swap(__mdspan_ebco& __x, __mdspan_ebco& __y)
  {
    swap(__x.__get<0>(), __y.__get<0>());
    swap(__x.__get<1>(), __y.__get<1>());
  }
};

template <class _Elem1, class _Elem2, class _Elem3>
struct _CCCL_DECLSPEC_EMPTY_BASES __mdspan_ebco<_Elem1, _Elem2, _Elem3>
    : __mdspan_ebco_impl<0, _Elem1>
    , __mdspan_ebco_impl<1, _Elem2>
    , __mdspan_ebco_impl<2, _Elem3>
{
  using __base1 = __mdspan_ebco_impl<0, _Elem1>;
  using __base2 = __mdspan_ebco_impl<1, _Elem2>;
  using __base3 = __mdspan_ebco_impl<2, _Elem3>;

  _CCCL_EXEC_CHECK_DISABLE
  _CCCL_TEMPLATE(class _Elem1_ = _Elem1, class _Elem2_ = _Elem2, class _Elem3_ = _Elem3)
  _CCCL_REQUIRES(is_default_constructible_v<_Elem1_> _CCCL_AND is_default_constructible_v<_Elem2_> _CCCL_AND
                   is_default_constructible_v<_Elem3_>)
  _CCCL_API constexpr __mdspan_ebco() noexcept(
    is_nothrow_default_constructible_v<_Elem1_> && is_nothrow_default_constructible_v<_Elem2_>
    && is_nothrow_default_constructible_v<_Elem3_>)
      : __base1()
      , __base2()
      , __base3()
  {}

  template <class _Arg1>
  static constexpr bool __is_constructible_from_one_arg =
    is_constructible_v<_Elem1, _Arg1> && is_default_constructible_v<_Elem2> && is_default_constructible_v<_Elem3>;

  template <class _Arg1>
  static constexpr bool __is_nothrow_constructible_from_one_arg =
    is_nothrow_constructible_v<_Elem1, _Arg1> && is_nothrow_default_constructible_v<_Elem2>
    && is_nothrow_default_constructible_v<_Elem3>;

  _CCCL_EXEC_CHECK_DISABLE
  _CCCL_TEMPLATE(class _Arg1)
  _CCCL_REQUIRES(__is_constructible_from_one_arg<_Arg1>)
  _CCCL_API constexpr __mdspan_ebco(_Arg1&& __arg1) noexcept(__is_nothrow_constructible_from_one_arg<_Arg1>)
      : __base1(::cuda::std::forward<_Arg1>(__arg1))
      , __base2()
      , __base3()
  {}

  template <class _Arg1, class _Arg2>
  static constexpr bool __is_constructible_from_two_args =
    is_constructible_v<_Elem1, _Arg1> && is_constructible_v<_Elem2, _Arg2> && is_default_constructible_v<_Elem3>;

  template <class _Arg1, class _Arg2>
  static constexpr bool __is_nothrow_constructible_from_two_args =
    is_nothrow_constructible_v<_Elem1, _Arg1> && is_nothrow_constructible_v<_Elem2, _Arg2>
    && is_nothrow_default_constructible_v<_Elem3>;

  _CCCL_EXEC_CHECK_DISABLE
  _CCCL_TEMPLATE(class _Arg1, class _Arg2)
  _CCCL_REQUIRES(__is_constructible_from_two_args<_Arg1, _Arg2>)
  _CCCL_API constexpr __mdspan_ebco(_Arg1&& __arg1,
                                    _Arg2&& __arg2) noexcept(__is_nothrow_constructible_from_two_args<_Arg1, _Arg2>)
      : __base1(::cuda::std::forward<_Arg1>(__arg1))
      , __base2(::cuda::std::forward<_Arg2>(__arg2))
      , __base3()
  {}

  template <class _Arg1, class _Arg2, class _Arg3>
  static constexpr bool __is_constructible_from_three_args =
    is_constructible_v<_Elem1, _Arg1> && is_constructible_v<_Elem2, _Arg2> && is_constructible_v<_Elem3, _Arg3>;

  template <class _Arg1, class _Arg2, class _Arg3>
  static constexpr bool __is_nothrow_constructible_from_three_args =
    is_nothrow_constructible_v<_Elem1, _Arg1> && is_nothrow_constructible_v<_Elem2, _Arg2>
    && is_nothrow_constructible_v<_Elem3, _Arg3>;

  _CCCL_EXEC_CHECK_DISABLE
  _CCCL_TEMPLATE(class _Arg1, class _Arg2, class _Arg3)
  _CCCL_REQUIRES(__is_constructible_from_three_args<_Arg1, _Arg2, _Arg3>)
  _CCCL_API constexpr __mdspan_ebco(_Arg1&& __arg1, _Arg2&& __arg2, _Arg3&& __arg3) noexcept(
    __is_nothrow_constructible_from_three_args<_Arg1, _Arg2, _Arg3>)
      : __base1(::cuda::std::forward<_Arg1>(__arg1))
      , __base2(::cuda::std::forward<_Arg2>(__arg2))
      , __base3(::cuda::std::forward<_Arg3>(__arg3))
  {}

  _CCCL_TEMPLATE(size_t _Index)
  _CCCL_REQUIRES((_Index < 3))
  [[nodiscard]] _CCCL_API constexpr decltype(auto) __get() noexcept
  {
    if constexpr (_Index == 0)
    {
      return static_cast<__base1*>(this)->__get();
    }
    else if constexpr (_Index == 1)
    {
      return static_cast<__base2*>(this)->__get();
    }
    else // if constexpr (_Index == 2)
    {
      return static_cast<__base3*>(this)->__get();
    }
    _CCCL_UNREACHABLE();
  }

  _CCCL_TEMPLATE(size_t _Index)
  _CCCL_REQUIRES((_Index < 3))
  [[nodiscard]] _CCCL_API constexpr decltype(auto) __get() const noexcept
  {
    if constexpr (_Index == 0)
    {
      return static_cast<const __base1*>(this)->__get();
    }
    else if constexpr (_Index == 1)
    {
      return static_cast<const __base2*>(this)->__get();
    }
    else // if constexpr (_Index == 2)
    {
      return static_cast<const __base3*>(this)->__get();
    }
    _CCCL_UNREACHABLE();
  }

  _CCCL_EXEC_CHECK_DISABLE
  _CCCL_API friend constexpr void swap(__mdspan_ebco& __x, __mdspan_ebco& __y)
  {
    swap(__x.__get<0>(), __y.__get<0>());
    swap(__x.__get<1>(), __y.__get<1>());
    swap(__x.__get<2>(), __y.__get<2>());
  }
};

_CCCL_END_NAMESPACE_CUDA_STD

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA_STD___MDSPAN_EMPTY_BASE_H
