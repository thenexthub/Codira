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

#pragma once

#include <uscl/__utility/immovable.h>

#include <uscl/experimental/execution.cuh>

#include "testing.cuh" // IWYU pragma: keep

namespace
{
struct _stopped_scheduler_attrs_t
{
  template <class _Env>
  _CCCL_HOST_DEVICE auto
  query(cudax_async::get_completion_scheduler_t<cudax_async::set_value_t>, const _Env& env) const noexcept
    -> decltype(cudax_async::get_completion_scheduler<cudax_async::set_value_t>(env, env))
  {
    return cudax_async::get_completion_scheduler<cudax_async::set_value_t>(env, env);
  }

  template <class _Env>
  _CCCL_HOST_DEVICE auto
  query(cudax_async::get_completion_scheduler_t<cudax_async::set_stopped_t>, const _Env& env) const noexcept
    -> decltype(cudax_async::get_completion_scheduler<cudax_async::set_stopped_t>(env, env))
  {
    return cudax_async::get_completion_scheduler<cudax_async::set_stopped_t>(env, env);
  }

  template <class _Env>
  _CCCL_HOST_DEVICE auto
  query(cudax_async::get_completion_domain_t<cudax_async::set_value_t>, const _Env& env) const noexcept
    -> decltype(cudax_async::get_completion_domain<cudax_async::set_value_t>(env, env))
  {
    return cudax_async::get_completion_domain<cudax_async::set_value_t>(env, env);
  }

  template <class _Env>
  _CCCL_HOST_DEVICE auto
  query(cudax_async::get_completion_domain_t<cudax_async::set_stopped_t>, const _Env& env) const noexcept
    -> decltype(cudax_async::get_completion_domain<cudax_async::set_stopped_t>(env, env))
  {
    return cudax_async::get_completion_domain<cudax_async::set_stopped_t>(env, env);
  }

  _CCCL_HOST_DEVICE static constexpr auto query(cudax_async::get_completion_behavior_t) noexcept
  {
    return cudax_async::completion_behavior::inline_completion;
  }
};

//! Scheduler that returns a sender that always completes with stopped.
struct stopped_scheduler : _stopped_scheduler_attrs_t
{
private:
  template <class Rcvr>
  struct _opstate_t : cuda::__immovable
  {
    using operation_state_concept = cudax_async::operation_state_t;

    Rcvr _rcvr;

    _CCCL_HOST_DEVICE void start() noexcept
    {
      cudax_async::set_stopped(static_cast<Rcvr&&>(_rcvr));
    }
  };

  struct _sndr_t
  {
    using sender_concept = cudax_async::sender_t;

    template <class Self>
    _CCCL_HOST_DEVICE static constexpr auto get_completion_signatures()
    {
      return cudax_async::completion_signatures<cudax_async::set_value_t(), cudax_async::set_stopped_t()>();
    }

    template <class Rcvr>
    _CCCL_HOST_DEVICE auto connect(Rcvr rcvr) const noexcept -> _opstate_t<Rcvr>
    {
      return {{}, static_cast<Rcvr&&>(rcvr)};
    }

    _CCCL_HOST_DEVICE auto get_env() const noexcept -> _stopped_scheduler_attrs_t
    {
      return {};
    }
  };

public:
  using scheduler_concept = cudax_async::scheduler_t;

  stopped_scheduler() = default;

  _CCCL_HOST_DEVICE auto schedule() const noexcept -> _sndr_t
  {
    return {};
  }

  _CCCL_HOST_DEVICE friend constexpr bool operator==(stopped_scheduler, stopped_scheduler) noexcept
  {
    return true;
  }

  _CCCL_HOST_DEVICE friend constexpr bool operator!=(stopped_scheduler, stopped_scheduler) noexcept
  {
    return false;
  }
};
} // namespace
