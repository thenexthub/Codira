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

#ifndef __CUDAX_EXECUTION_TYPE_TRAITS
#define __CUDAX_EXECUTION_TYPE_TRAITS

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/std/__type_traits/type_list.h>

#include <uscl/experimental/__detail/type_traits.cuh> // IWYU pragma: export

#include <uscl/experimental/__execution/prologue.cuh>

namespace cuda::experimental::execution
{
template <class _Ret, class... _Args>
using __fn_t _CCCL_NODEBUG_ALIAS = _Ret(_Args...);

template <class _Ret, class... _Args>
using __fn_ptr_t _CCCL_NODEBUG_ALIAS = _Ret (*)(_Args...);

template <class _Ty>
using __cref_t _CCCL_NODEBUG_ALIAS = _Ty const&;

using __cp _CCCL_NODEBUG_ALIAS    = ::cuda::std::__type_self;
using __cpclr _CCCL_NODEBUG_ALIAS = ::cuda::std::__type_quote1<__cref_t>;

} // namespace cuda::experimental::execution

#include <uscl/experimental/__execution/epilogue.cuh>

#endif // __CUDAX_EXECUTION_TYPE_TRAITS
