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

#include <uscl/std/type_traits>

#include <uscl/experimental/execution.cuh>

#include <testing.cuh>

namespace execution = cuda::experimental::execution;

struct with_get_execution_policy_const_lvalue
{
  execution::any_execution_policy pol_ = execution::seq;

  const execution::any_execution_policy& get_execution_policy() const noexcept
  {
    return pol_;
  }
};
C2H_TEST("Can call get_execution_policy on a type with a get_execution_policy method that returns a const lvalue",
         "[execution][policies]")
{
  with_get_execution_policy_const_lvalue val{};
  auto&& res = cuda::experimental::execution::get_execution_policy(val);
  STATIC_REQUIRE(cuda::std::is_same_v<decltype(res), execution::any_execution_policy&&>);
  CHECK(val.pol_ == res);
}

struct with_get_execution_policy_rvalue
{
  execution::any_execution_policy pol_{};

  execution::any_execution_policy get_execution_policy() const noexcept
  {
    return pol_;
  }
};
C2H_TEST("Can call get_execution_policy on a type with a get_execution_policy method returns an rvalue",
         "[execution][policies]")
{
  with_get_execution_policy_rvalue val{};
  auto&& res = cuda::experimental::execution::get_execution_policy(val);
  STATIC_REQUIRE(cuda::std::is_same_v<decltype(res), execution::any_execution_policy&&>);
  CHECK(val.pol_ == res);
}

struct with_get_execution_policy_non_const
{
  execution::any_execution_policy pol_{};

  execution::any_execution_policy get_execution_policy() noexcept
  {
    return pol_;
  }
};
C2H_TEST("Cannot call get_execution_policy on a type with a non-const get_execution_policy method",
         "[execution][policies]")
{
  STATIC_REQUIRE(!::cuda::std::is_invocable_v<cuda::experimental::execution::get_execution_policy_t,
                                              const with_get_execution_policy_non_const&>);
}

struct env_with_query_const_ref
{
  execution::any_execution_policy pol_{};

  execution::any_execution_policy query(cuda::experimental::execution::get_execution_policy_t) const noexcept
  {
    return pol_;
  }
};
C2H_TEST("Can call get_execution_policy on an env with a get_execution_policy query that returns a const lvalue",
         "[execution][policies]")
{
  env_with_query_const_ref val{};
  auto&& res = cuda::experimental::execution::get_execution_policy(val);
  STATIC_REQUIRE(cuda::std::is_same_v<decltype(res), execution::any_execution_policy&&>);
  CHECK(val.pol_ == res);
}

struct env_with_query_rvalue
{
  execution::any_execution_policy pol_{};

  execution::any_execution_policy query(cuda::experimental::execution::get_execution_policy_t) const noexcept
  {
    return pol_;
  }
};
C2H_TEST("Can call get_execution_policy on an env with a get_execution_policy query that returns an rvalue",
         "[execution][policies]")
{
  env_with_query_rvalue val{};
  auto&& res = cuda::experimental::execution::get_execution_policy(val);
  STATIC_REQUIRE(cuda::std::is_same_v<decltype(res), execution::any_execution_policy&&>);
  CHECK(val.pol_ == res);
}

struct env_with_query_non_const
{
  execution::any_execution_policy pol_{};

  execution::any_execution_policy query(cuda::experimental::execution::get_execution_policy_t) noexcept
  {
    return pol_;
  }
};
C2H_TEST("Cannot call get_execution_policy on an env with a non-const query", "[execution][policies]")
{
  STATIC_REQUIRE(
    !::cuda::std::is_invocable_v<cuda::experimental::execution::get_execution_policy_t, const env_with_query_non_const&>);
}

struct env_with_query_and_method
{
  execution::any_execution_policy pol_{};

  execution::any_execution_policy get_execution_policy() const noexcept
  {
    return pol_;
  }

  execution::any_execution_policy query(cuda::experimental::execution::get_execution_policy_t) const noexcept
  {
    return pol_;
  }
};
C2H_TEST("Can call get_execution_policy on a type with both get_execution_policy and query", "[execution][policies]")
{
  env_with_query_and_method val{};
  auto&& res = cuda::experimental::execution::get_execution_policy(val);
  STATIC_REQUIRE(cuda::std::is_same_v<decltype(res), execution::any_execution_policy&&>);
  CHECK(val.pol_ == res);
}
