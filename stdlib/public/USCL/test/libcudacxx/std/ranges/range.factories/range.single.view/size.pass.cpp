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

// static constexpr size_t size() noexcept;

#include <uscl/std/cassert>
#include <uscl/std/ranges>

#include "test_macros.h"

__host__ __device__ constexpr bool test()
{
  {
    auto sv = cuda::std::ranges::single_view<int>(42);
    unused(sv);
    assert(sv.size() == 1);

    static_assert(cuda::std::is_same_v<decltype(sv.size()), size_t>);
    static_assert(noexcept(sv.size()));
  }
  {
    const auto sv = cuda::std::ranges::single_view<int>(42);
    assert(sv.size() == 1);

    static_assert(cuda::std::is_same_v<decltype(sv.size()), size_t>);
    static_assert(noexcept(sv.size()));
  }
  {
    auto sv = cuda::std::ranges::single_view<int>(42);
    assert(cuda::std::ranges::size(sv) == 1);

    static_assert(cuda::std::is_same_v<decltype(cuda::std::ranges::size(sv)), size_t>);
    static_assert(noexcept(cuda::std::ranges::size(sv)));
  }
  {
    const auto sv = cuda::std::ranges::single_view<int>(42);
    assert(cuda::std::ranges::size(sv) == 1);

    static_assert(cuda::std::is_same_v<decltype(cuda::std::ranges::size(sv)), size_t>);
    static_assert(noexcept(cuda::std::ranges::size(sv)));
  }

  // Test that it's static.
  {
    assert(cuda::std::ranges::single_view<int>::size() == 1);

    static_assert(cuda::std::is_same_v<decltype(cuda::std::ranges::single_view<int>::size()), size_t>);
    static_assert(noexcept(cuda::std::ranges::single_view<int>::size()));
  }

  return true;
}

int main(int, char**)
{
  test();
  static_assert(test());

  return 0;
}
