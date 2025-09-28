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

#ifndef __CUDA___EXECUTION_REQUIRE_H
#define __CUDA___EXECUTION_REQUIRE_H

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/std/__concepts/concept_macros.h>
#include <uscl/std/__execution/env.h>
#include <uscl/std/__type_traits/is_base_of.h>
#include <uscl/std/__type_traits/is_empty.h>

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA_EXECUTION

class __requirement
{};

struct __get_requirements_t
{
  _CCCL_EXEC_CHECK_DISABLE
  _CCCL_TEMPLATE(class _Env)
  _CCCL_REQUIRES(::cuda::std::execution::__queryable_with<_Env, __get_requirements_t>)
  [[nodiscard]] _CCCL_NODEBUG_API constexpr auto operator()(const _Env& __env) const noexcept
  {
    static_assert(noexcept(__env.query(*this)));
    return __env.query(*this);
  }

  [[nodiscard]]
  _CCCL_NODEBUG_API static constexpr auto query(::cuda::std::execution::forwarding_query_t) noexcept -> bool
  {
    return true;
  }
};

_CCCL_GLOBAL_CONSTANT auto __get_requirements = __get_requirements_t{};

template <class... _Requirements>
[[nodiscard]] _CCCL_NODEBUG_API auto require(_Requirements...)
{
  static_assert((::cuda::std::is_base_of_v<__requirement, _Requirements> && ...),
                "Only requirements can be passed to require");
  static_assert((::cuda::std::is_empty_v<_Requirements> && ...), "Stateful requirements are not implemented");

  // clang < 19 doesn't like this code
  // since the only requirements we currently allow are in determinism.h and
  // all of them are stateless, let's ignore incoming parameters
  ::cuda::std::execution::env<_Requirements...> __env{};

  return ::cuda::std::execution::prop{__get_requirements_t{}, __env};
}

_CCCL_END_NAMESPACE_CUDA_EXECUTION

#include <uscl/std/__cccl/epilogue.h>

#endif // __CUDA___EXECUTION_REQUIRE_H
