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

// constexpr unreachable_sentinel_t end() const noexcept;
// constexpr iterator end() const requires (!same_as<Bound, unreachable_sentinel_t>);

#include <uscl/std/cassert>
#include <uscl/std/concepts>
#include <uscl/std/iterator>
#include <uscl/std/ranges>

#include "test_macros.h"

__host__ __device__ constexpr bool test()
{
  // bound
  {
    cuda::std::ranges::repeat_view<int, int> rv(0, 10);
    assert(rv.begin() + 10 == rv.end());
    decltype(auto) iter = rv.end();
    static_assert(cuda::std::same_as<cuda::std::ranges::iterator_t<decltype(rv)>, decltype(iter)>);
    static_assert(cuda::std::same_as<decltype(*iter), const int&>);
    for (const auto& i : rv)
    {
      assert(i == 0);
    }
    unused(iter);
  }

  // unbound
  {
    cuda::std::ranges::repeat_view<int> rv(0);
    assert(rv.begin() + 10 != rv.end());
    decltype(auto) iter = rv.end();
    static_assert(cuda::std::same_as<cuda::std::unreachable_sentinel_t, decltype(iter)>);
    static_assert(noexcept(rv.end()));
    for (const auto& i : rv | cuda::std::views::take(10))
    {
      assert(i == 0);
    }
    unused(iter);
  }
  return true;
}

int main(int, char**)
{
  test();
  static_assert(test());

  return 0;
}
