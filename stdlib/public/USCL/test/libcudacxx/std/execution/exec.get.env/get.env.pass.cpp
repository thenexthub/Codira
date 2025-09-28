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

#include <uscl/std/execution>

// all other includes follow after <cuda/std/execution>
#include <uscl/std/__type_traits/is_same.h>

#include "test_macros.h"

[[maybe_unused]] _CCCL_GLOBAL_CONSTANT struct query1_t
{
} query1{};
[[maybe_unused]] _CCCL_GLOBAL_CONSTANT struct query2_t
{
} query2{};

struct an_env_t
{
  __host__ __device__ constexpr auto query(query1_t) const noexcept -> int
  {
    return 42;
  }

  __host__ __device__ constexpr auto query(query2_t) const noexcept -> double
  {
    return 3.14;
  }
};

struct env_provider
{
  __host__ __device__ constexpr auto get_env() const noexcept -> decltype(auto)
  {
    return an_env_t{};
  }
};

struct none_such_t
{};

__host__ __device__ TEST_CONSTEXPR_CXX20 bool test()
{
  env_provider provider;
  [[maybe_unused]] auto&& env = cuda::std::execution::get_env(provider);

  static_assert(cuda::std::is_same_v<decltype(env), an_env_t&&>, "");
  static_assert(cuda::std::is_same_v<decltype(cuda::std::execution::get_env), const cuda::std::execution::get_env_t>,
                "");
  static_assert(noexcept(cuda::std::execution::get_env(provider)), "");

  [[maybe_unused]] auto&& env2 = cuda::std::execution::get_env(none_such_t{});
  static_assert(cuda::std::is_same_v<decltype(env2), cuda::std::execution::env<>&&>, "");

  return true;
}

int main(int, char**)
{
  test();

#if TEST_STD_VER >= 2020
  static_assert(test());
#endif // TEST_STD_VER >= 2020

  return 0;
}
