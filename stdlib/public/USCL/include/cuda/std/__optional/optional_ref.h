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

#ifndef _CUDA_STD___OPTIONAL_OPTIONAL_REF_H
#define _CUDA_STD___OPTIONAL_OPTIONAL_REF_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/std/__concepts/concept_macros.h>
#include <uscl/std/__concepts/invocable.h>
#include <uscl/std/__functional/invoke.h>
#include <uscl/std/__fwd/optional.h>
#include <uscl/std/__memory/addressof.h>
#include <uscl/std/__optional/bad_optional_access.h>
#include <uscl/std/__optional/nullopt.h>
#include <uscl/std/__optional/optional_base.h>
#include <uscl/std/__type_traits/decay.h>
#include <uscl/std/__type_traits/is_array.h>
#include <uscl/std/__type_traits/is_constructible.h>
#include <uscl/std/__type_traits/is_convertible.h>
#include <uscl/std/__type_traits/is_copy_constructible.h>
#include <uscl/std/__type_traits/is_reference.h>
#include <uscl/std/__type_traits/is_same.h>
#include <uscl/std/__type_traits/reference_constructs_from_temporary.h>
#include <uscl/std/__type_traits/reference_converts_from_temporary.h>
#include <uscl/std/__type_traits/remove_cvref.h>
#include <uscl/std/__type_traits/remove_reference.h>
#include <uscl/std/__utility/declval.h>
#include <uscl/std/__utility/forward.h>
#include <uscl/std/__utility/in_place.h>
#include <uscl/std/__utility/move.h>
#include <uscl/std/__utility/swap.h>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA_STD

#ifdef CCCL_ENABLE_OPTIONAL_REF
template <class _Tp>
class optional<_Tp&>
{
private:
  using __raw_type     = remove_reference_t<_Tp>;
  __raw_type* __value_ = nullptr;

  _CCCL_TEMPLATE(class _Ref, class _Arg)
  _CCCL_REQUIRES(is_constructible_v<_Ref, _Arg>)
  [[nodiscard]] _CCCL_API static constexpr _Ref __make_reference(_Arg&& __arg) noexcept
  {
    static_assert(is_reference_v<_Ref>, "optional<T&>: make-reference requires a reference as argument");
    return _Ref(::cuda::std::forward<_Arg>(__arg));
  }

  // Needed to interface with optional<T>
  template <class>
  friend struct __optional_storage_base;

  [[nodiscard]] _CCCL_API constexpr _Tp& __get() noexcept
  {
    return *__value_;
  }
  [[nodiscard]] _CCCL_API constexpr const _Tp& __get() const noexcept
  {
    return *__value_;
  }

#  if defined(_CCCL_BUILTIN_REFERENCE_CONSTRUCTS_FROM_TEMPORARY)
  template <class _Up>
  static constexpr bool __from_temporary = reference_constructs_from_temporary_v<_Tp&, _Up>;
#  else
  template <class _Up>
  static constexpr bool __from_temporary = false;
#  endif // !_CCCL_BUILTIN_REFERENCE_CONSTRUCTS_FROM_TEMPORARY

public:
  using value_type = __raw_type&;

  _CCCL_API constexpr optional() noexcept {}
  _CCCL_HIDE_FROM_ABI constexpr optional(const optional&) noexcept = default;
  _CCCL_HIDE_FROM_ABI constexpr optional(optional&&) noexcept      = default;
  _CCCL_API constexpr optional(nullopt_t) noexcept {}

  _CCCL_TEMPLATE(class _Arg)
  _CCCL_REQUIRES(is_constructible_v<_Tp&, _Arg> _CCCL_AND(!__from_temporary<_Arg>))
  _CCCL_API explicit constexpr optional(in_place_t, _Arg&& __arg) noexcept
      : __value_(::cuda::std::addressof(__make_reference<_Tp&>(::cuda::std::forward<_Arg>(__arg))))
  {}

  _CCCL_TEMPLATE(class _Up)
  _CCCL_REQUIRES(
    (!__is_std_optional_v<decay_t<_Up>>) _CCCL_AND is_convertible_v<_Up, _Tp&> _CCCL_AND(!__from_temporary<_Up>))
  _CCCL_API constexpr optional(_Up&& __u) noexcept(noexcept(static_cast<_Tp&>(::cuda::std::declval<_Up>())))
      : __value_(::cuda::std::addressof(static_cast<_Tp&>(::cuda::std::forward<_Up>(__u))))
  {}

  _CCCL_TEMPLATE(class _Up)
  _CCCL_REQUIRES((!__is_std_optional_v<decay_t<_Up>>) _CCCL_AND(!is_convertible_v<_Up, _Tp&>)
                   _CCCL_AND is_constructible_v<_Tp&, _Up> _CCCL_AND(!__from_temporary<_Up>))
  _CCCL_API explicit constexpr optional(_Up&& __u) noexcept(noexcept(static_cast<_Tp&>(::cuda::std::declval<_Up>())))
      : __value_(::cuda::std::addressof(static_cast<_Tp&>(::cuda::std::forward<_Up>(__u))))
  {}

  _CCCL_TEMPLATE(class _Up)
  _CCCL_REQUIRES((!__is_std_optional_v<decay_t<_Up>>) _CCCL_AND __from_temporary<_Up>)
  _CCCL_API constexpr optional(_Up&&) = delete;

  _CCCL_TEMPLATE(class _Up)
  _CCCL_REQUIRES(is_convertible_v<_Up&, _Tp&> _CCCL_AND(!__from_temporary<_Up&>))
  _CCCL_API constexpr optional(optional<_Up>& __u) noexcept(noexcept(static_cast<_Tp&>(::cuda::std::declval<_Up&>())))
      : __value_(__u.has_value() ? ::cuda::std::addressof(static_cast<_Tp&>(__u.value())) : nullptr)
  {}

  _CCCL_TEMPLATE(class _Up)
  _CCCL_REQUIRES(
    (!is_convertible_v<_Up&, _Tp&>) _CCCL_AND is_constructible_v<_Tp&, _Up&> _CCCL_AND(!__from_temporary<_Up&>))
  _CCCL_API explicit constexpr optional(optional<_Up>& __u) noexcept(
    noexcept(static_cast<_Tp&>(::cuda::std::declval<_Up&>())))
      : __value_(__u.has_value() ? ::cuda::std::addressof(static_cast<_Tp&>(__u.value())) : nullptr)
  {}

  _CCCL_TEMPLATE(class _Up)
  _CCCL_REQUIRES(__from_temporary<_Up&>)
  _CCCL_API constexpr optional(optional<_Up>&) = delete;

  _CCCL_TEMPLATE(class _Up)
  _CCCL_REQUIRES(is_convertible_v<const _Up&, _Tp&> _CCCL_AND(!__from_temporary<const _Up&>))
  _CCCL_API constexpr optional(const optional<_Up>& __u) noexcept(
    noexcept(static_cast<_Tp&>(::cuda::std::declval<const _Up&>())))
      : __value_(__u.has_value() ? ::cuda::std::addressof(static_cast<_Tp&>(__u.value())) : nullptr)
  {}

  _CCCL_TEMPLATE(class _Up)
  _CCCL_REQUIRES((!is_convertible_v<const _Up&, _Tp&>) _CCCL_AND is_constructible_v<_Tp&, const _Up&> _CCCL_AND(
    !__from_temporary<const _Up&>))
  _CCCL_API explicit constexpr optional(const optional<_Up>& __u) noexcept(
    noexcept(static_cast<_Tp&>(::cuda::std::declval<const _Up&>())))
      : __value_(__u.has_value() ? ::cuda::std::addressof(static_cast<_Tp&>(__u.value())) : nullptr)
  {}

  _CCCL_TEMPLATE(class _Up)
  _CCCL_REQUIRES(__from_temporary<const _Up&>)
  _CCCL_API constexpr optional(const optional<_Up>&) = delete;

  _CCCL_TEMPLATE(class _Up)
  _CCCL_REQUIRES(is_convertible_v<_Up, _Tp&> _CCCL_AND(!__from_temporary<_Up>))
  _CCCL_API constexpr optional(optional<_Up>&& __u) noexcept(noexcept(static_cast<_Tp&>(::cuda::std::declval<_Up>())))
      : __value_(
          __u.has_value() ? ::cuda::std::addressof(static_cast<_Tp&>(::cuda::std::forward<_Up>(__u.value()))) : nullptr)
  {}

  _CCCL_TEMPLATE(class _Up)
  _CCCL_REQUIRES(
    (!is_convertible_v<_Up, _Tp&>) _CCCL_AND is_constructible_v<_Tp&, _Up> _CCCL_AND(!__from_temporary<_Up>))
  _CCCL_API explicit constexpr optional(optional<_Up>&& __u) noexcept(
    noexcept(static_cast<_Tp&>(::cuda::std::declval<_Up>())))
      : __value_(
          __u.has_value() ? ::cuda::std::addressof(static_cast<_Tp&>(::cuda::std::forward<_Up>(__u.value()))) : nullptr)
  {}

  _CCCL_TEMPLATE(class _Up)
  _CCCL_REQUIRES(__from_temporary<_Up>)
  _CCCL_API constexpr optional(optional<_Up>&&) = delete;

  _CCCL_TEMPLATE(class _Up)
  _CCCL_REQUIRES(is_convertible_v<const _Up, _Tp&> _CCCL_AND(!__from_temporary<const _Up>))
  _CCCL_API constexpr optional(const optional<_Up>&& __u) noexcept(
    noexcept(static_cast<_Tp&>(::cuda::std::declval<const _Up>())))
      : __value_(__u.has_value() ? ::cuda::std::addressof(static_cast<_Tp&>(__u.value())) : nullptr)
  {}

  _CCCL_TEMPLATE(class _Up)
  _CCCL_REQUIRES((!is_convertible_v<const _Up, _Tp&>) _CCCL_AND is_constructible_v<_Tp&, const _Up> _CCCL_AND(
    !__from_temporary<const _Up>))
  _CCCL_API explicit constexpr optional(const optional<_Up>&& __u) noexcept(
    noexcept(static_cast<_Tp&>(::cuda::std::declval<const _Up>())))
      : __value_(__u.has_value() ? ::cuda::std::addressof(static_cast<_Tp&>(__u.value())) : nullptr)
  {}

  _CCCL_TEMPLATE(class _Up)
  _CCCL_REQUIRES(__from_temporary<const _Up>)
  _CCCL_API constexpr optional(const optional<_Up>&&) = delete;

  _CCCL_HIDE_FROM_ABI constexpr optional& operator=(const optional&) noexcept = default;
  _CCCL_HIDE_FROM_ABI constexpr optional& operator=(optional&&) noexcept      = default;

  _CCCL_API constexpr optional& operator=(nullopt_t) noexcept
  {
    __value_ = nullptr;
    return *this;
  }

  _CCCL_TEMPLATE(class _Up = _Tp)
  _CCCL_REQUIRES(is_constructible_v<_Tp&, _Up> _CCCL_AND(!__from_temporary<_Up>))
  _CCCL_API constexpr _Tp& emplace(_Up&& __u) noexcept(noexcept(static_cast<_Tp&>(::cuda::std::forward<_Up>(__u))))
  {
    __value_ = ::cuda::std::addressof(static_cast<_Tp&>(::cuda::std::forward<_Up>(__u)));
    return *__value_;
  }

  _CCCL_API constexpr void swap(optional& __rhs) noexcept
  {
    return ::cuda::std::swap(__value_, __rhs.__value_);
  }

  _CCCL_API constexpr _Tp* operator->() const noexcept
  {
    _CCCL_ASSERT(__value_ != nullptr, "optional operator-> called on a disengaged value");
    return __value_;
  }

  _CCCL_API constexpr _Tp& operator*() const noexcept
  {
    _CCCL_ASSERT(__value_ != nullptr, "optional operator* called on a disengaged value");
    return *__value_;
  }

  _CCCL_API constexpr explicit operator bool() const noexcept
  {
    return __value_ != nullptr;
  }

  _CCCL_API constexpr bool has_value() const noexcept
  {
    return __value_ != nullptr;
  }

  _CCCL_API constexpr _Tp& value() const noexcept
  {
    if (__value_ != nullptr)
    {
      return *__value_;
    }
    else
    {
      __throw_bad_optional_access();
    }
  }

  template <class _Up>
  _CCCL_API constexpr remove_cvref_t<_Tp> value_or(_Up&& __v) const
  {
    static_assert(is_copy_constructible_v<_Tp>, "optional<T&>::value_or: T must be copy constructible");
    static_assert(is_convertible_v<_Up, _Tp>, "optional<T&>::value_or: U must be convertible to T");
    return __value_ != nullptr ? *__value_ : static_cast<_Tp>(::cuda::std::forward<_Up>(__v));
  }

  template <class _Func>
  _CCCL_API constexpr auto and_then(_Func&& __f) const
  {
    using _Up = invoke_result_t<_Func, _Tp&>;
    static_assert(__is_std_optional_v<remove_cvref_t<_Up>>,
                  "optional<T&>::and_then: Result of f(value()) must be a specialization of std::optional");
    if (__value_ != nullptr)
    {
      return ::cuda::std::invoke(::cuda::std::forward<_Func>(__f), *__value_);
    }
    return remove_cvref_t<_Up>();
  }

  template <class _Func>
  _CCCL_API constexpr auto transform(_Func&& __f) const
  {
    using _Up = invoke_result_t<_Func, _Tp&>;
    static_assert(!is_array_v<_Up>, "optional<T&>::transform: Result of f(value()) should not be an Array");
    static_assert(!is_same_v<_Up, in_place_t>,
                  "optional<T&>::transform: Result of f(value()) should not be std::in_place_t");
    static_assert(!is_same_v<_Up, nullopt_t>,
                  "optional<T&>::transform: Result of f(value()) should not be std::nullopt_t");
    if (__value_ != nullptr)
    {
      if constexpr (is_lvalue_reference_v<_Up>)
      {
        return optional<_Up>(::cuda::std::invoke(::cuda::std::forward<_Func>(__f), *__value_));
      }
      else
      {
        return optional<_Up>(__optional_construct_from_invoke_tag{}, ::cuda::std::forward<_Func>(__f), *__value_);
      }
    }
    return optional<_Up>();
  }

  _CCCL_TEMPLATE(class _Func)
  _CCCL_REQUIRES(invocable<_Func>)
  _CCCL_API constexpr optional or_else(_Func&& __f) const
  {
    using _Up = invoke_result_t<_Func>;
    static_assert(is_same_v<remove_cvref_t<_Up>, optional>,
                  "optional<T&>::or_else: Result of f() should be the same type as this optional");
    if (__value_ != nullptr)
    {
      return *this;
    }
    return ::cuda::std::forward<_Func>(__f)();
  }

  _CCCL_API constexpr void reset() noexcept
  {
    __value_ = nullptr;
  }
};

#endif // CCCL_ENABLE_OPTIONAL_REF

_CCCL_END_NAMESPACE_CUDA_STD

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA_STD___OPTIONAL_OPTIONAL_REF_H
