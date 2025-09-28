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

#ifndef _CUDA___UTILITY_BASIC_ANY_BASIC_ANY_FROM_H
#define _CUDA___UTILITY_BASIC_ANY_BASIC_ANY_FROM_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/__utility/__basic_any/basic_any_fwd.h>
#include <uscl/std/__type_traits/decay.h>
#include <uscl/std/__utility/declval.h>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA

//!
//! `__basic_any_from`
//!
//! \brief This function is for use in the thunks in an interface to get
//! a pointer or a reference to the full `__basic_any` object.
//!
template <template <class...> class _Interface, class _Super>
[[nodiscard]] _CCCL_NODEBUG_API auto __basic_any_from(_Interface<_Super>&& __self) noexcept -> __basic_any<_Super>&&
{
  return static_cast<__basic_any<_Super>&&>(__self);
}

template <template <class...> class _Interface, class _Super>
[[nodiscard]] _CCCL_NODEBUG_API auto __basic_any_from(_Interface<_Super>& __self) noexcept -> __basic_any<_Super>&
{
  return static_cast<__basic_any<_Super>&>(__self);
}

template <template <class...> class _Interface, class _Super>
[[nodiscard]] _CCCL_NODEBUG_API auto __basic_any_from(_Interface<_Super> const& __self) noexcept
  -> __basic_any<_Super> const&
{
  return static_cast<__basic_any<_Super> const&>(__self);
}

template <template <class...> class _Interface>
[[nodiscard]] _CCCL_API auto __basic_any_from(_Interface<> const&) noexcept -> __basic_any<_Interface<>> const&
{
  // This overload is selected when called from the thunk of an unspecialized
  // interface; e.g., `icat<>` rather than `icat<ialley_cat<>>`. The thunks of
  // unspecialized interfaces are never called, they just need to exist.
  _CCCL_UNREACHABLE();
}

template <template <class...> class _Interface, class _Super>
[[nodiscard]] _CCCL_NODEBUG_API auto __basic_any_from(_Interface<_Super>* __self) noexcept -> __basic_any<_Super>*
{
  return static_cast<__basic_any<_Super>*>(__self);
}

template <template <class...> class _Interface, class _Super>
[[nodiscard]] _CCCL_NODEBUG_API auto __basic_any_from(_Interface<_Super> const* __self) noexcept
  -> __basic_any<_Super> const*
{
  return static_cast<__basic_any<_Super> const*>(__self);
}

template <template <class...> class _Interface>
[[nodiscard]] _CCCL_API auto __basic_any_from(_Interface<> const*) noexcept -> __basic_any<_Interface<>> const*
{
  // See comment above about the use of `__basic_any_from` in the thunks of
  // unspecialized interfaces.
  _CCCL_UNREACHABLE();
}

template <class _CvInterface>
using __cvref_basic_any_from_t = decltype(::cuda::__basic_any_from(::cuda::std::declval<_CvInterface>()));

template <class _CvInterface>
using __basic_any_from_t = ::cuda::std::decay_t<__cvref_basic_any_from_t<_CvInterface>>;
_CCCL_END_NAMESPACE_CUDA

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA___UTILITY_BASIC_ANY_BASIC_ANY_FROM_H
