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

#ifndef _CUDA_STD___RANGES_ENABLE_VIEW_H
#define _CUDA_STD___RANGES_ENABLE_VIEW_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/std/__concepts/derived_from.h>
#include <uscl/std/__concepts/same_as.h>
#include <uscl/std/__type_traits/enable_if.h>
#include <uscl/std/__type_traits/is_class.h>
#include <uscl/std/__type_traits/remove_cv.h>
#include <uscl/std/__type_traits/void_t.h>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_RANGES

struct view_base
{};

#if _CCCL_HAS_CONCEPTS()

template <class _Derived>
  requires is_class_v<_Derived> && same_as<_Derived, remove_cv_t<_Derived>>
class view_interface;

#else // ^^^ _CCCL_HAS_CONCEPTS() ^^^ / vvv !_CCCL_HAS_CONCEPTS() vvv

template <class _Derived, enable_if_t<is_class_v<_Derived> && same_as<_Derived, remove_cv_t<_Derived>>, int> = 0>
class view_interface;

#endif // ^^^ !_CCCL_HAS_CONCEPTS() ^^^

_CCCL_TEMPLATE(class _Op, class _Yp)
_CCCL_REQUIRES(is_convertible_v<_Op*, view_interface<_Yp>*>)
_CCCL_API inline void __is_derived_from_view_interface(const _Op*, const view_interface<_Yp>*);

#if _CCCL_HAS_CONCEPTS()

template <class _Tp>
inline constexpr bool enable_view = derived_from<_Tp, view_base> || requires {
  ::cuda::std::ranges::__is_derived_from_view_interface((_Tp*) nullptr, (_Tp*) nullptr);
};

#else // ^^^ _CCCL_HAS_CONCEPTS() ^^^ / vvv !_CCCL_HAS_CONCEPTS() vvv

template <class _Tp, class = void>
inline constexpr bool enable_view = derived_from<_Tp, view_base>;

template <class _Tp>
inline constexpr bool
  enable_view<_Tp,
              void_t<decltype(::cuda::std::ranges::__is_derived_from_view_interface((_Tp*) nullptr, (_Tp*) nullptr))>> =
    true;
#endif // ^^^ !_CCCL_HAS_CONCEPTS() ^^^

_CCCL_END_NAMESPACE_RANGES

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA_STD___RANGES_ENABLE_VIEW_H
