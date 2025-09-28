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

#ifndef _CUDA___UTILITY_BASIC_ANY_ACCESS_H
#define _CUDA___UTILITY_BASIC_ANY_ACCESS_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/__type_traits/is_specialization_of.h>
#include <uscl/__utility/__basic_any/basic_any_fwd.h>
#include <uscl/__utility/__basic_any/conversions.h>
#include <uscl/__utility/__basic_any/interfaces.h>
#include <uscl/std/__concepts/concept_macros.h>
#include <uscl/std/__type_traits/remove_const.h>
#include <uscl/std/__type_traits/remove_cvref.h>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA

//!
//! __basic_any_access
//!
struct __basic_any_access
{
  template <class _Interface>
  _CCCL_NODEBUG_API static auto __make() noexcept -> __basic_any<_Interface>
  {
    return __basic_any<_Interface>{};
  }

  _CCCL_TEMPLATE(class _SrcCvAny, class _DstInterface)
  _CCCL_REQUIRES(__any_castable_to<_SrcCvAny, __basic_any<_DstInterface>>)
  _CCCL_NODEBUG_API static auto __cast_to(_SrcCvAny&& __from, __basic_any<_DstInterface>& __to) noexcept(
    noexcept(__to.__convert_from(static_cast<_SrcCvAny&&>(__from)))) -> void
  {
    static_assert(__is_specialization_of_v<::cuda::std::remove_cvref_t<_SrcCvAny>, __basic_any>);
    __to.__convert_from(static_cast<_SrcCvAny&&>(__from));
  }

  _CCCL_TEMPLATE(class _SrcCvAny, class _DstInterface)
  _CCCL_REQUIRES(__any_castable_to<_SrcCvAny*, __basic_any<_DstInterface>>)
  _CCCL_NODEBUG_API static auto
  __cast_to(_SrcCvAny* __from, __basic_any<_DstInterface>& __to) noexcept(noexcept(__to.__convert_from(__from))) -> void
  {
    static_assert(__is_specialization_of_v<::cuda::std::remove_const_t<_SrcCvAny>, __basic_any>);
    __to.__convert_from(__from);
  }

  template <class _Interface>
  _CCCL_NODEBUG_API static auto __get_vptr(__basic_any<_Interface> const& __self) noexcept -> __vptr_for<_Interface>
  {
    return __self.__get_vptr();
  }

  template <class _Interface>
  _CCCL_NODEBUG_API static auto __get_optr(__basic_any<_Interface>& __self) noexcept -> void*
  {
    return __self.__get_optr();
  }

  template <class _Interface>
  _CCCL_NODEBUG_API static auto __get_optr(__basic_any<_Interface> const& __self) noexcept -> void const*
  {
    return __self.__get_optr();
  }
};

_CCCL_END_NAMESPACE_CUDA

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA___UTILITY_BASIC_ANY_ACCESS_H
