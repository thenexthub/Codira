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

// friend constexpr iter_difference_t<W> operator-(const iterator& x, const sentinel& y)
//   requires sized_sentinel_for<Bound, W>;
// friend constexpr iter_difference_t<W> operator-(const sentinel& x, const iterator& y)
//   requires sized_sentinel_for<Bound, W>;

#include <uscl/std/cassert>
#include <uscl/std/ranges>

#include "../types.h"
#include "test_iterators.h"
#include "test_macros.h"

template <class T>
_CCCL_CONCEPT MinusInvocable =
  _CCCL_REQUIRES_EXPR((T), cuda::std::ranges::iota_view<T, IntSentinelWith<T>> io)(io.end() - io.begin());

__host__ __device__ constexpr bool test()
{
  int buffer[8] = {1, 2, 3, 4, 5, 6, 7, 8};

  {
    auto outIter = random_access_iterator<int*>(buffer);
    cuda::std::ranges::iota_view<random_access_iterator<int*>, IntSentinelWith<random_access_iterator<int*>>> io(
      outIter, IntSentinelWith<random_access_iterator<int*>>(cuda::std::ranges::next(outIter, 8)));
    auto iter = io.begin();
    auto sent = io.end();
    assert(iter - sent == -8);
    assert(sent - iter == 8);
  }
  {
    auto outIter = random_access_iterator<int*>(buffer);
    const cuda::std::ranges::iota_view<random_access_iterator<int*>, IntSentinelWith<random_access_iterator<int*>>> io(
      outIter, IntSentinelWith<random_access_iterator<int*>>(cuda::std::ranges::next(outIter, 8)));
    const auto iter = io.begin();
    const auto sent = io.end();
    assert(iter - sent == -8);
    assert(sent - iter == 8);
  }

  {
    // The minus operator requires that "W" is an input_or_output_iterator.
    static_assert(!MinusInvocable<int>);
  }

  return true;
}

int main(int, char**)
{
  test();
  static_assert(test(), "");

  return 0;
}
