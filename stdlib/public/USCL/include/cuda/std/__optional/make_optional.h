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

#ifndef _CUDA_STD___OPTIONAL_MAKE_OPTIONAL_H
#define _CUDA_STD___OPTIONAL_MAKE_OPTIONAL_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/std/__optional/optional.h>
#include <uscl/std/__type_traits/decay.h>
#include <uscl/std/__type_traits/is_reference.h>
#include <uscl/std/__type_traits/is_same.h>
#include <uscl/std/__utility/forward.h>
#include <uscl/std/__utility/in_place.h>
#include <uscl/std/initializer_list>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA_STD

_CCCL_TEMPLATE(class _Tp = nullopt_t::__secret_tag, class _Up)
_CCCL_REQUIRES(is_same_v<_Tp, nullopt_t::__secret_tag>)
_CCCL_API constexpr optional<decay_t<_Up>> make_optional(_Up&& __v)
{
  return optional<decay_t<_Up>>(::cuda::std::forward<_Up>(__v));
}

_CCCL_TEMPLATE(class _Tp, class... _Args)
_CCCL_REQUIRES((!is_reference_v<_Tp>) )
_CCCL_API constexpr optional<_Tp> make_optional(_Args&&... __args)
{
  return optional<_Tp>(in_place, ::cuda::std::forward<_Args>(__args)...);
}

_CCCL_TEMPLATE(class _Tp, class _Up, class... _Args)
_CCCL_REQUIRES((!is_reference_v<_Tp>) )
_CCCL_API constexpr optional<_Tp> make_optional(initializer_list<_Up> __il, _Args&&... __args)
{
  return optional<_Tp>(in_place, __il, ::cuda::std::forward<_Args>(__args)...);
}

_CCCL_END_NAMESPACE_CUDA_STD

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA_STD___OPTIONAL_MAKE_OPTIONAL_H
