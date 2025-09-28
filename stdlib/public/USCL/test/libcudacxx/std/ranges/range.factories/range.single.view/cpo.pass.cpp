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

// cuda::std::views::single

#include <uscl/std/cassert>
#include <uscl/std/concepts>
#include <uscl/std/ranges>
#include <uscl/std/utility>

#include "MoveOnly.h"
#include "test_macros.h"

// Can't invoke without arguments.
static_assert(!cuda::std::is_invocable_v<decltype((cuda::std::views::single))>);
// Can invoke with a move-only type.
static_assert(cuda::std::is_invocable_v<decltype((cuda::std::views::single)), MoveOnly>);

__host__ __device__ constexpr bool test()
{
  // Lvalue.
  {
    int x            = 42;
    decltype(auto) v = cuda::std::views::single(x);
    static_assert(cuda::std::same_as<decltype(v), cuda::std::ranges::single_view<int>>);
    assert(v.size() == 1);
    assert(v.front() == x);
  }

  // Prvalue.
  {
    decltype(auto) v = cuda::std::views::single(42);
    static_assert(cuda::std::same_as<decltype(v), cuda::std::ranges::single_view<int>>);
    assert(v.size() == 1);
    assert(v.front() == 42);
  }

  // Const lvalue.
  {
    const int x      = 42;
    decltype(auto) v = cuda::std::views::single(x);
    static_assert(cuda::std::same_as<decltype(v), cuda::std::ranges::single_view<int>>);
    assert(v.size() == 1);
    assert(v.front() == x);
  }

  // Xvalue.
  {
    int x            = 42;
    decltype(auto) v = cuda::std::views::single(cuda::std::move(x));
    static_assert(cuda::std::same_as<decltype(v), cuda::std::ranges::single_view<int>>);
    assert(v.size() == 1);
    assert(v.front() == x);
  }

  return true;
}

int main(int, char**)
{
  test();
#if defined(_CCCL_BUILTIN_ADDRESSOF)
  static_assert(test());
#endif // _CCCL_BUILTIN_ADDRESSOF

  return 0;
}
