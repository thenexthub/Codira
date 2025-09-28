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

#include <system_error>

#include "common/checked_receiver.cuh"
#include "common/dummy_scheduler.cuh"

#if _CCCL_COMPILER(GCC, <, 12)
// suppress buggy warning on older gcc versions
_CCCL_DIAG_SUPPRESS_GCC("-Wmissing-field-initializers")
#endif

namespace ex = cuda::experimental::execution;

namespace
{
struct test_domain
{};

C2H_TEST("simple test of just sender factory", "[just]")
{
  auto sndr  = ex::just(42);
  using Sndr = decltype(sndr);
  STATIC_REQUIRE(ex::sender<Sndr>);

  auto op = ex::connect(sndr, checked_value_receiver{42});
  ex::start(op);

  STATIC_CHECK(ex::get_completion_behavior<Sndr>() == ex::completion_behavior::inline_completion);
  STATIC_CHECK(!cudax::__callable<ex::get_completion_scheduler_t<ex::set_value_t>, ex::env_of_t<Sndr>>);
  STATIC_CHECK(!cudax::__callable<ex::get_completion_domain_t<ex::set_value_t>, ex::env_of_t<Sndr>>);

  constexpr auto sch = dummy_scheduler<test_domain>{};
  constexpr auto env = ex::prop{ex::get_scheduler, sch};
  STATIC_CHECK(ex::get_completion_scheduler<ex::set_value_t>(ex::get_env(sndr), env) == sch);
  STATIC_CHECK(
    cuda::std::is_same_v<decltype(ex::get_completion_domain<ex::set_value_t>(ex::get_env(sndr), env)), test_domain>);
}

C2H_TEST("simple test of just_error sender factory", "[just]")
{
  auto ec    = ::std::errc::invalid_argument;
  auto sndr  = ex::just_error(ec);
  using Sndr = decltype(sndr);
  STATIC_REQUIRE(ex::sender<Sndr>);

  auto op = ex::connect(sndr, checked_error_receiver{ec});
  ex::start(op);

  STATIC_REQUIRE(ex::get_completion_behavior<Sndr>() == ex::completion_behavior::inline_completion);
  STATIC_REQUIRE(!cudax::__callable<ex::get_completion_scheduler_t<ex::set_error_t>, ex::env_of_t<Sndr>>);
  STATIC_REQUIRE(!cudax::__callable<ex::get_completion_domain_t<ex::set_error_t>, ex::env_of_t<Sndr>>);

  constexpr auto sch = dummy_scheduler<test_domain>{};
  constexpr auto env = ex::prop{ex::get_scheduler, sch};
  STATIC_CHECK(ex::get_completion_scheduler<ex::set_error_t>(ex::get_env(sndr), env) == sch);
  STATIC_CHECK(
    cuda::std::is_same_v<decltype(ex::get_completion_domain<ex::set_error_t>(ex::get_env(sndr), env)), test_domain>);
}

C2H_TEST("simple test of just_stopped sender factory", "[just]")
{
  auto sndr  = ex::just_stopped();
  using Sndr = decltype(sndr);
  STATIC_REQUIRE(ex::sender<Sndr>);

  auto op = ex::connect(sndr, checked_stopped_receiver{});
  ex::start(op);

  STATIC_REQUIRE(ex::get_completion_behavior<Sndr>() == ex::completion_behavior::inline_completion);
  STATIC_REQUIRE(!cudax::__callable<ex::get_completion_scheduler_t<ex::set_stopped_t>, ex::env_of_t<Sndr>>);
  STATIC_REQUIRE(!cudax::__callable<ex::get_completion_domain_t<ex::set_stopped_t>, ex::env_of_t<Sndr>>);

  constexpr auto sch = dummy_scheduler<test_domain>{};
  constexpr auto env = ex::prop{ex::get_scheduler, sch};
  STATIC_CHECK(ex::get_completion_scheduler<ex::set_stopped_t>(ex::get_env(sndr), env) == sch);
  STATIC_CHECK(
    cuda::std::is_same_v<decltype(ex::get_completion_domain<ex::set_stopped_t>(ex::get_env(sndr), env)), test_domain>);
}
} // anonymous namespace
