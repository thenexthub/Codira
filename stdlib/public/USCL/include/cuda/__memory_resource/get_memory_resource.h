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

#ifndef _CUDA___MEMORY_RESOURCE_GET_MEMORY_RESOURCE_CUH
#define _CUDA___MEMORY_RESOURCE_GET_MEMORY_RESOURCE_CUH

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/__memory_resource/properties.h>
#include <uscl/__memory_resource/resource.h>
#include <uscl/std/__concepts/equality_comparable.h>
#include <uscl/std/__execution/env.h>
#include <uscl/std/__type_traits/is_same.h>
#include <uscl/std/__type_traits/remove_cvref.h>
#include <uscl/stream_ref>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA_MR

struct __get_memory_resource_t;

template <class _Tp>
_CCCL_CONCEPT __has_member_get_resource = _CCCL_REQUIRES_EXPR((_Tp), const _Tp& __t)(
  requires(resource<::cuda::std::remove_cvref_t<decltype(__t.get_memory_resource())>>));

template <class _Env>
_CCCL_CONCEPT __has_query_get_memory_resource = _CCCL_REQUIRES_EXPR((_Env))(
  requires(!__has_member_get_resource<_Env>),
  requires(
    resource<
      ::cuda::std::remove_cvref_t<::cuda::std::execution::__query_result_t<const _Env&, __get_memory_resource_t>>>));

//! @brief `__get_memory_resource_t` is a customization point object that queries a type `T` for an associated memory
//! resource
struct __get_memory_resource_t
{
  _CCCL_EXEC_CHECK_DISABLE
  _CCCL_TEMPLATE(class _Tp)
  _CCCL_REQUIRES(__has_member_get_resource<_Tp>)
  [[nodiscard]] _CCCL_API constexpr decltype(auto) operator()(const _Tp& __t) const noexcept
  {
    static_assert(noexcept(__t.get_memory_resource()), "get_memory_resource must be noexcept");
    return __t.get_memory_resource();
  }

  _CCCL_EXEC_CHECK_DISABLE
  _CCCL_TEMPLATE(class _Env)
  _CCCL_REQUIRES(__has_query_get_memory_resource<_Env>)
  [[nodiscard]] _CCCL_API constexpr decltype(auto) operator()(const _Env& __env) const noexcept
  {
    static_assert(noexcept(__env.query(*this)), "get_memory_resource_t query must be noexcept");
    return __env.query(*this);
  }
};

_CCCL_GLOBAL_CONSTANT auto __get_memory_resource = __get_memory_resource_t{};

using get_memory_resource_t = __get_memory_resource_t;

_CCCL_GLOBAL_CONSTANT auto get_memory_resource = get_memory_resource_t{};

_CCCL_END_NAMESPACE_CUDA_MR

#include <uscl/std/__cccl/epilogue.h>

#endif //_CUDAX__MEMORY_RESOURCE_GET_MEMORY_RESOURCE_CUH
