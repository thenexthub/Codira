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

#include <uscl/experimental/execution.cuh>

#include "common/dummy_scheduler.cuh"
#include "testing.cuh" // IWYU pragma: keep

namespace async = ::cuda::experimental::execution;

struct not_a_receiver
{};

struct a_receiver
{
  using receiver_concept   = async::receiver_t;
  a_receiver(a_receiver&&) = default;

  void set_value(int) && noexcept {}
  void set_stopped() && noexcept {}
};

C2H_TEST("tests for the receiver concepts", "[concepts]")
{
  static_assert(!async::receiver<not_a_receiver>);
  static_assert(async::receiver<a_receiver>);

  using yes_completions = async::completion_signatures<async::set_value_t(int), async::set_stopped_t()>;
  static_assert(async::receiver_of<a_receiver, yes_completions>);

  using no_completions = async::
    completion_signatures<async::set_value_t(int), async::set_stopped_t(), async::set_error_t(::std::exception_ptr)>;
  static_assert(!async::receiver_of<a_receiver, no_completions>);
}

struct not_a_sender
{};

struct a_sender
{
  using sender_concept = async::sender_t;

  template <class _Self>
  static constexpr auto get_completion_signatures()
  {
    return async::completion_signatures<async::set_value_t(int), async::set_stopped_t()>{};
  }
};

struct non_constexpr_complsigs
{
  using sender_concept = async::sender_t;

  template <class _Self, class...>
  _CCCL_HOST_DEVICE static auto get_completion_signatures()
  {
    return async::completion_signatures<async::set_value_t(int), async::set_stopped_t()>{};
  }
};

C2H_TEST("tests for the sender concepts", "[concepts]")
{
  static_assert(!async::sender<not_a_sender>);
  static_assert(async::sender<a_sender>);

  static_assert(async::sender_in<a_sender>);
  static_assert(async::sender_in<a_sender, async::env<>>);

  static_assert(async::sender<non_constexpr_complsigs>);
  static_assert(!async::sender_in<non_constexpr_complsigs>);
  static_assert(!async::sender_in<non_constexpr_complsigs, async::env<>>);

  [[maybe_unused]] auto read_env = async::read_env(async::get_scheduler);
  using read_env_t               = decltype(read_env);
  static_assert(async::sender<read_env_t>);
  static_assert(!async::sender_in<read_env_t>);
  static_assert(!async::sender_in<read_env_t, async::env<>>);
  static_assert(async::sender_in<read_env_t, async::prop<async::get_scheduler_t, dummy_scheduler<>>>);
}
