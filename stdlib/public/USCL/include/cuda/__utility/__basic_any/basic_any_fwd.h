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

#ifndef _CUDA___UTILITY_BASIC_ANY_FWD_H
#define _CUDA___UTILITY_BASIC_ANY_FWD_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/std/__type_traits/integral_constant.h>
#include <uscl/std/__type_traits/type_list.h>
#include <uscl/std/cstddef> // for max_align_t
#include <uscl/std/cstdint> // for uint8_t

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA

template <class _Interface>
struct __ireference;

template <class _Interface>
struct _CCCL_TYPE_VISIBILITY_DEFAULT __basic_any;

template <class _Interface>
struct _CCCL_DECLSPEC_EMPTY_BASES __basic_any<__ireference<_Interface>>;

template <class _Interface>
struct __basic_any<_Interface*>;

template <class _Interface>
struct __basic_any<_Interface&>;

template <auto _Value>
using __constant = ::cuda::std::integral_constant<decltype(_Value), _Value>;

template <class _InterfaceOrModel, class... _VirtualFnsOrOverrides>
struct __overrides_list;

template <class _InterfaceOrModel, auto... _VirtualFnsOrOverrides>
using __overrides_for = __overrides_list<_InterfaceOrModel, __constant<_VirtualFnsOrOverrides>...>;

template <class _Interface, auto... _Mbrs>
struct _CCCL_DECLSPEC_EMPTY_BASES __basic_vtable;

struct __rtti_base;

struct __rtti;

template <size_t NbrBases>
struct __rtti_ex;

template <class...>
struct __extends;

template <template <class...> class, class = __extends<>, size_t = 0, size_t = 0>
struct __basic_interface;

template <class _Interface, class... _Super>
using __rebind_interface _CCCL_NODEBUG_ALIAS = typename _Interface::template __rebind<_Super...>;

struct __iunknown;

template <class...>
struct __iset_;

template <class...>
struct __iset_vptr;

template <class...>
struct __imovable;

template <class...>
struct __icopyable;

template <class...>
struct __iequality_comparable;

template <class... _Tp>
using __tag _CCCL_NODEBUG_ALIAS = ::cuda::std::__type_list_ptr<_Tp...>;

template <auto...>
struct __ctag_;

template <auto... _Is>
using __ctag _CCCL_NODEBUG_ALIAS = __ctag_<_Is...>*;

constexpr size_t __word                       = sizeof(void*);
constexpr size_t __default_small_object_size  = 3 * __word;
constexpr size_t __default_small_object_align = alignof(::cuda::std::max_align_t);

using __make_type_list _CCCL_NODEBUG_ALIAS = ::cuda::std::__type_quote<::cuda::std::__type_list>;

[[noreturn]] _CCCL_API void __throw_bad_any_cast();

enum class __vtable_kind : uint8_t
{
  __normal,
  __rtti,
};

inline constexpr uint8_t __basic_any_version = 0;

template <class _Interface>
extern _Interface __remove_ireference_v; // specialized in interfaces.cuh

template <class _Interface>
using __remove_ireference_t _CCCL_NODEBUG_ALIAS = decltype(__remove_ireference_v<_Interface>);

_CCCL_END_NAMESPACE_CUDA

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA___UTILITY_BASIC_ANY_FWD_H
