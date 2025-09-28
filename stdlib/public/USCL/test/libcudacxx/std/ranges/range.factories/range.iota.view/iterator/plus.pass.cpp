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

// friend constexpr iterator operator+(iterator i, difference_type n)
//   requires advanceable<W>;
// friend constexpr iterator operator+(difference_type n, iterator i)
//   requires advanceable<W>;

#include <uscl/std/cassert>
#include <uscl/std/ranges>

#include "../types.h"
#include "test_macros.h"

__host__ __device__ constexpr bool test()
{
  // When "_Start" is signed integer like.
  {
    cuda::std::ranges::iota_view<int> io(0);
    auto iter1 = io.begin();
    auto iter2 = io.begin();
    assert(iter1 == iter2);
    assert(iter1 + 5 != iter2);
    assert(iter1 + 5 == cuda::std::ranges::next(iter2, 5));

    static_assert(cuda::std::is_reference_v<decltype(iter2 += 5)>);
  }

  // When "_Start" is not integer like.
  {
    static_assert(cuda::std::totally_ordered<SomeInt>);
    cuda::std::ranges::iota_view io(SomeInt(0));
    auto iter1 = io.begin();
    auto iter2 = io.begin();
    assert(iter1 == iter2);
    assert(iter1 + 5 != iter2);
    assert(iter1 + 5 == cuda::std::ranges::next(iter2, 5));

    static_assert(cuda::std::is_reference_v<decltype(iter2 += 5)>);
  }

  // When "_Start" is unsigned integer like and n is greater than or equal to zero.
  {
    cuda::std::ranges::iota_view<unsigned> io(0);
    auto iter1 = io.begin();
    auto iter2 = io.begin();
    assert(iter1 == iter2);
    assert(iter1 + 5 != iter2);
    assert(iter1 + 5 == cuda::std::ranges::next(iter2, 5));

    static_assert(cuda::std::is_reference_v<decltype(iter2 += 5)>);
  }
  {
    cuda::std::ranges::iota_view<unsigned> io(0);
    auto iter1 = io.begin();
    auto iter2 = io.begin();
    assert(iter1 == iter2);
    assert(iter1 + 0 == iter2);
  }

  // When "_Start" is unsigned integer like and n is less than zero.
  {
    cuda::std::ranges::iota_view<unsigned> io(0);
    auto iter1 = io.begin();
    auto iter2 = io.begin();
    assert(iter1 == iter2);
    assert(iter1 + (-5) != iter2);
    assert(iter1 + (-5) == cuda::std::ranges::prev(iter2, 5));

    static_assert(cuda::std::is_reference_v<decltype(iter2 += (-5))>);
  }

  return true;
}

int main(int, char**)
{
  test();
  static_assert(test(), "");

  return 0;
}
