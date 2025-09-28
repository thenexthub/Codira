/*
 *
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 * Date: Wednesday, July 24, 2024.
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

#ifndef __CUDAX_DETAIL_TYPE_TRAITS_CUH
#define __CUDAX_DETAIL_TYPE_TRAITS_CUH

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/std/__concepts/concept_macros.h>
#include <uscl/std/__type_traits/decay.h>
#include <uscl/std/__type_traits/enable_if.h>
#include <uscl/std/__type_traits/integral_constant.h>
#include <uscl/std/__type_traits/is_callable.h>
#include <uscl/std/__type_traits/is_constructible.h>
#include <uscl/std/__type_traits/is_copy_constructible.h>
#include <uscl/std/__type_traits/is_move_constructible.h>
#include <uscl/std/__type_traits/is_nothrow_constructible.h>
#include <uscl/std/__type_traits/is_nothrow_copy_constructible.h>
#include <uscl/std/__type_traits/is_nothrow_move_constructible.h>
#include <uscl/std/__type_traits/is_same.h>
#include <uscl/std/__type_traits/is_valid_expansion.h>

#include <uscl/std/__cccl/prologue.h>

namespace cuda::experimental
{
using ::cuda::std::decay_t;

template <class _Ty>
using __declfn = _Ty && (*) () noexcept;

template <class _Ty, class _Uy>
_CCCL_CONCEPT __same_as = ::cuda::std::_IsSame<_Ty, _Uy>::value;

template <class _Ty, class _Uy>
_CCCL_CONCEPT __not_same_as = !::cuda::std::_IsSame<_Ty, _Uy>::value;

template <class _Ty, class... _Us>
_CCCL_CONCEPT __one_of = (__same_as<_Ty, _Us> || ...);

template <class _Ty, class... _Us>
_CCCL_CONCEPT __none_of = (__not_same_as<_Ty, _Us> && ...);

#if _CCCL_HAS_CONCEPTS()

template <template <class...> class _Fn, class... _Ts>
_CCCL_CONCEPT __is_instantiable_with = requires { typename _Fn<_Ts...>; };

template <class _Fn, class... _As>
_CCCL_CONCEPT __callable = requires(__declfn<_Fn> __fn, __declfn<_As>... __as) { __fn()(__as()...); };

#else // ^^^ _CCCL_HAS_CONCEPTS() ^^^ / vvv !_CCCL_HAS_CONCEPTS() vvv

template <template <class...> class _Fn, class... _Ts>
_CCCL_CONCEPT __is_instantiable_with = ::cuda::std::_IsValidExpansion<_Fn, _Ts...>::value;

template <class _Fn, class... _As>
_CCCL_CONCEPT __callable = ::cuda::std::__is_callable_v<_Fn, _As...>;

#endif // !_CCCL_HAS_CONCEPTS()

template <class _Fn, class... _As>
_CCCL_CONCEPT __constructible = ::cuda::std::is_constructible_v<_Fn, _As...>;

template <class... _As>
_CCCL_CONCEPT __decay_copyable = (::cuda::std::is_constructible_v<decay_t<_As>, _As> && ...);

template <class... _As>
_CCCL_CONCEPT __movable = (::cuda::std::is_move_constructible_v<_As> && ...);

template <class... _As>
_CCCL_CONCEPT __copyable = (::cuda::std::is_copy_constructible_v<_As> && ...);

#if _CCCL_HOST_COMPILATION()
template <class _Fn, class... _As>
_CCCL_CONCEPT __nothrow_callable = ::cuda::std::__is_nothrow_callable_v<_Fn, _As...>;

template <class _Ty, class... _As>
_CCCL_CONCEPT __nothrow_constructible = ::cuda::std::is_nothrow_constructible_v<_Ty, _As...>;

template <class... _As>
_CCCL_CONCEPT __nothrow_decay_copyable = (::cuda::std::is_nothrow_constructible_v<decay_t<_As>, _As> && ...);

template <class... _As>
_CCCL_CONCEPT __nothrow_movable = (::cuda::std::is_nothrow_move_constructible_v<_As> && ...);

template <class... _As>
_CCCL_CONCEPT __nothrow_copyable = (::cuda::std::is_nothrow_copy_constructible_v<_As> && ...);
#else // ^^^ _CCCL_HOST_COMPILATION() ^^^ / vvv !_CCCL_HOST_COMPILATION() vvv
// There are no exceptions in device code:
template <class _Fn, class... _As>
_CCCL_CONCEPT __nothrow_callable = ::cuda::std::__is_callable_v<_Fn, _As...>;

template <class _Ty, class... _As>
_CCCL_CONCEPT __nothrow_constructible = ::cuda::std::is_constructible_v<_Ty, _As...>;

template <class... _As>
_CCCL_CONCEPT __nothrow_decay_copyable = __decay_copyable<_As...>;

template <class... _As>
_CCCL_CONCEPT __nothrow_movable = (::cuda::std::is_move_constructible_v<_As> && ...);

template <class... _As>
_CCCL_CONCEPT __nothrow_copyable = (::cuda::std::is_copy_constructible_v<_As> && ...);
#endif // ^^^ !_CCCL_HOST_COMPILATION() ^^^

template <class... _As>
using __nothrow_decay_copyable_t _CCCL_NODEBUG_ALIAS = ::cuda::std::bool_constant<__nothrow_decay_copyable<_As...>>;

using ::cuda::std::__call_result_t;

} // namespace cuda::experimental

#include <uscl/std/__cccl/epilogue.h>

#endif // __CUDAX_DETAIL_TYPE_TRAITS_CUH
