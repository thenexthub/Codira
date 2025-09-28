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

// friend constexpr iterator operator-(iterator i, difference_type n);
// friend constexpr difference_type operator-(const iterator& x, const iterator& y);

#include <uscl/std/cassert>
#include <uscl/std/concepts>
#include <uscl/std/cstdint>
#include <uscl/std/ranges>

#include "test_macros.h"

__host__ __device__ constexpr bool test()
{
  // <iterator> - difference_type
  {
    using Iter = cuda::std::ranges::iterator_t<cuda::std::ranges::repeat_view<int>>;
    cuda::std::ranges::repeat_view<int> v(0);
    Iter iter = v.begin() + 10;
    assert(iter - 5 == v.begin() + 5);
    static_assert(cuda::std::same_as<decltype(iter - 5), Iter>);
  }

  // <iterator> - <iterator>
  {
    // unbound
    {
      cuda::std::ranges::repeat_view<int> v(0);
      auto iter1 = v.begin() + 10;
      auto iter2 = v.begin() + 5;
      assert(iter1 - iter2 == 5);
      static_assert(cuda::std::same_as<decltype(iter1 - iter2), ptrdiff_t>);
    }

    // bound && signed bound sentinel
    {
      cuda::std::ranges::repeat_view<int, int> v(0, 20);
      auto iter1 = v.begin() + 10;
      auto iter2 = v.begin() + 5;
      assert(iter1 - iter2 == 5);
      static_assert(cuda::std::same_as<decltype(iter1 - iter2), int>);
    }

    // bound && unsigned bound sentinel
    {
      cuda::std::ranges::repeat_view<int, unsigned> v(0, 20);
      auto iter1 = v.begin() + 10;
      auto iter2 = v.begin() + 5;
      assert(iter1 - iter2 == 5);
      static_assert(sizeof(decltype(iter1 - iter2)) > sizeof(unsigned));
    }
  }

  return true;
}

int main(int, char**)
{
  test();
  static_assert(test());

  return 0;
}
