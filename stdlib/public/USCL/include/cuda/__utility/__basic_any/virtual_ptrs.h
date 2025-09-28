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

#ifndef _CUDA___UTILITY_BASIC_ANY_VIRTUAL_PTRS_H
#define _CUDA___UTILITY_BASIC_ANY_VIRTUAL_PTRS_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/__utility/__basic_any/basic_any_fwd.h>
#include <uscl/__utility/__basic_any/interfaces.h>
#include <uscl/std/__exception/terminate.h>
#include <uscl/std/__utility/typeid.h>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA

struct __base_vptr
{
  __base_vptr() = default;

  _CCCL_NODEBUG_API constexpr __base_vptr(__rtti_base const* __vptr) noexcept
      : __vptr_(__vptr)
  {}

  template <class _VTable>
  [[nodiscard]] _CCCL_NODEBUG_API explicit constexpr operator _VTable const*() const noexcept
  {
    auto const* __vptr = static_cast<_VTable const*>(__vptr_);
    _CCCL_ASSERT(_CCCL_TYPEID(_VTable) == *__vptr->__typeid_, "bad vtable cast detected");
    return __vptr;
  }

  [[nodiscard]] _CCCL_NODEBUG_API explicit constexpr operator bool() const noexcept
  {
    return __vptr_ != nullptr;
  }

  [[nodiscard]] _CCCL_NODEBUG_API constexpr auto operator->() const noexcept -> __rtti_base const*
  {
    return __vptr_;
  }

#if !defined(_CCCL_NO_THREE_WAY_COMPARISON)
  bool operator==(__base_vptr const& __other) const noexcept = default;
#else // ^^^ !_CCCL_NO_THREE_WAY_COMPARISON ^^^ / vvv _CCCL_NO_THREE_WAY_COMPARISON vvv
  [[nodiscard]] _CCCL_API friend constexpr auto operator==(__base_vptr __lhs, __base_vptr __rhs) noexcept -> bool
  {
    return __lhs.__vptr_ == __rhs.__vptr_;
  }

  [[nodiscard]] _CCCL_API friend constexpr auto operator!=(__base_vptr __lhs, __base_vptr __rhs) noexcept -> bool
  {
    return !(__lhs == __rhs);
  }
#endif // _CCCL_NO_THREE_WAY_COMPARISON

  __rtti_base const* __vptr_{};
};

_CCCL_END_NAMESPACE_CUDA

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA___UTILITY_BASIC_ANY_VIRTUAL_PTRS_H
