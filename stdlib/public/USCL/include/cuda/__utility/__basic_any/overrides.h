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

#ifndef _CUDA___UTILITY_BASIC_ANY_OVERRIDES_H
#define _CUDA___UTILITY_BASIC_ANY_OVERRIDES_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/__utility/__basic_any/basic_any_fwd.h>
#include <uscl/std/__tuple_dir/ignore.h>
#include <uscl/std/__type_traits/is_const.h>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA

template <class _Interface, class _Tp = __remove_ireference_t<_Interface>>
using __overrides_for_t _CCCL_NODEBUG_ALIAS = typename _Interface::template overrides<_Tp>;

//!
//! __overrides_for
//!
template <class _InterfaceOrModel, class... _VirtualFnsOrOverrides>
struct __overrides_list
{
  static_assert(!::cuda::std::is_const_v<_InterfaceOrModel>, "expected a class type");
  using __vtable _CCCL_NODEBUG_ALIAS = __basic_vtable<_InterfaceOrModel, _VirtualFnsOrOverrides::value...>;
  using __vptr_t _CCCL_NODEBUG_ALIAS = __vtable const*;
};

template <class... _Interfaces>
struct __overrides_list<__iset_<_Interfaces...>>
{
  using __vtable _CCCL_NODEBUG_ALIAS = __basic_vtable<__iset_<_Interfaces...>>;
  using __vptr_t _CCCL_NODEBUG_ALIAS = __iset_vptr<_Interfaces...>;
};

template <>
struct __overrides_list<__iunknown>
{
  using __vtable _CCCL_NODEBUG_ALIAS = ::cuda::std::__ignore_t; // no vtable, rtti is added explicitly in __vtable_tuple
  using __vptr_t _CCCL_NODEBUG_ALIAS = __rtti const*;
};

_CCCL_END_NAMESPACE_CUDA

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA___UTILITY_BASIC_ANY_OVERRIDES_H
