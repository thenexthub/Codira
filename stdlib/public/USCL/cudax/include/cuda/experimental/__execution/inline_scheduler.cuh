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

#ifndef __CUDAX_EXECUTION_INLINE_SCHEDULER
#define __CUDAX_EXECUTION_INLINE_SCHEDULER

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/__utility/immovable.h>

#include <uscl/experimental/__execution/completion_behavior.cuh>
#include <uscl/experimental/__execution/completion_signatures.cuh>
#include <uscl/experimental/__execution/cpos.cuh>
#include <uscl/experimental/__execution/domain.cuh>
#include <uscl/experimental/__execution/env.cuh>
#include <uscl/experimental/__execution/fwd.cuh>
#include <uscl/experimental/__execution/utility.cuh>

#include <uscl/experimental/__execution/prologue.cuh>

namespace cuda::experimental::execution
{
//! Scheduler that returns a sender that always completes inline (successfully).
struct _CCCL_TYPE_VISIBILITY_DEFAULT inline_scheduler : __inln_attrs_t<set_value_t>
{
private:
  struct _CCCL_TYPE_VISIBILITY_DEFAULT __attrs_t : __inln_attrs_t<set_value_t>
  {};

  template <class _Rcvr>
  struct _CCCL_TYPE_VISIBILITY_DEFAULT __opstate_t : __immovable
  {
    using operation_state_concept = operation_state_t;

    _CCCL_API constexpr void start() noexcept
    {
      set_value(static_cast<_Rcvr&&>(__rcvr));
    }

    _Rcvr __rcvr;
  };

public:
  using scheduler_concept = scheduler_t;

  struct _CCCL_TYPE_VISIBILITY_DEFAULT __sndr_t
  {
    using sender_concept = sender_t;

    template <class Self>
    [[nodiscard]] _CCCL_API static constexpr auto get_completion_signatures() noexcept
    {
      return completion_signatures<set_value_t()>{};
    }

    template <class _Rcvr>
    [[nodiscard]] _CCCL_API constexpr auto connect(_Rcvr __rcvr) const noexcept -> __opstate_t<_Rcvr>
    {
      return {{}, static_cast<_Rcvr&&>(__rcvr)};
    }

    [[nodiscard]] _CCCL_API static constexpr auto get_env() noexcept -> __attrs_t
    {
      return {};
    }
  };

  [[nodiscard]] _CCCL_API constexpr auto schedule() const noexcept -> __sndr_t
  {
    return {};
  }

  [[nodiscard]] _CCCL_API friend constexpr bool operator==(inline_scheduler, inline_scheduler) noexcept
  {
    return true;
  }

  [[nodiscard]] _CCCL_API friend constexpr bool operator!=(inline_scheduler, inline_scheduler) noexcept
  {
    return false;
  }
};

} // namespace cuda::experimental::execution

#include <uscl/experimental/__execution/epilogue.cuh>

#endif // __CUDAX_EXECUTION_INLINE_SCHEDULER
