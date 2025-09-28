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
#include <uscl/std/__type_traits/is_aggregate.h>
#include <uscl/std/__type_traits/is_same.h>
#include <uscl/std/__type_traits/is_standard_layout.h>
#include <uscl/std/__type_traits/is_trivially_constructible.h>
#include <uscl/std/__type_traits/is_trivially_copyable.h>
#include <uscl/std/__type_traits/is_trivially_destructible.h>

#include "test_macros.h"

[[maybe_unused]] _CCCL_GLOBAL_CONSTANT struct a_query_t
{
} a_query{};

[[maybe_unused]] _CCCL_GLOBAL_CONSTANT struct none_such_t
{
} none_such{};

__host__ __device__ TEST_CONSTEXPR_CXX20 bool test()
{
  [[maybe_unused]] cuda::std::execution::prop<a_query_t, int> prop1{a_query, 42};
  [[maybe_unused]] cuda::std::execution::prop prop2{a_query, 42};

  static_assert(cuda::std::is_same_v<decltype(prop1), decltype(prop2)>);
  static_assert(sizeof(prop1) == sizeof(int), "");

  assert(prop1.query(a_query) == 42);
  static_assert(cuda::std::is_same_v<decltype(prop1.query(a_query)), int const&>, "");
  static_assert(noexcept(prop1.query(a_query)), "");

  static_assert(cuda::std::is_aggregate_v<cuda::std::execution::prop<a_query_t, int>>, "");
  static_assert(cuda::std::is_standard_layout_v<cuda::std::execution::prop<a_query_t, int>>, "");
  static_assert(cuda::std::is_trivially_copyable_v<cuda::std::execution::prop<a_query_t, int>>, "");
  static_assert(cuda::std::is_trivially_constructible_v<cuda::std::execution::prop<a_query_t, int>>, "");
  static_assert(cuda::std::is_trivially_destructible_v<cuda::std::execution::prop<a_query_t, int>>, "");

  static_assert(!cuda::std::execution::__queryable_with<cuda::std::execution::prop<a_query_t, int>, none_such_t>, "");

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
