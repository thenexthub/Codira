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

// friend constexpr iter_difference_t<I> operator-(
//   const counted_iterator& x, default_sentinel_t);
// friend constexpr iter_difference_t<I> operator-(
//   default_sentinel_t, const counted_iterator& y);

#include <uscl/std/iterator>

#include "test_iterators.h"
#include "test_macros.h"

__host__ __device__ constexpr bool test()
{
  int buffer[8] = {1, 2, 3, 4, 5, 6, 7, 8};

  {
    cuda::std::counted_iterator iter(random_access_iterator<int*>{buffer}, 8);
    assert(iter - cuda::std::default_sentinel == -8);
    assert(cuda::std::default_sentinel - iter == 8);
    assert(iter.count() == 8);

    static_assert(
      cuda::std::is_same_v<decltype(iter - cuda::std::default_sentinel), cuda::std::iter_difference_t<int*>>);
    static_assert(
      cuda::std::is_same_v<decltype(cuda::std::default_sentinel - iter), cuda::std::iter_difference_t<int*>>);
  }
  {
    const cuda::std::counted_iterator iter(random_access_iterator<int*>{buffer}, 8);
    assert(iter - cuda::std::default_sentinel == -8);
    assert(cuda::std::default_sentinel - iter == 8);
    assert(iter.count() == 8);

    static_assert(
      cuda::std::is_same_v<decltype(iter - cuda::std::default_sentinel), cuda::std::iter_difference_t<int*>>);
    static_assert(
      cuda::std::is_same_v<decltype(cuda::std::default_sentinel - iter), cuda::std::iter_difference_t<int*>>);
  }
  {
    cuda::std::counted_iterator iter(forward_iterator<int*>{buffer}, 8);
    assert(iter - cuda::std::default_sentinel == -8);
    assert(cuda::std::default_sentinel - iter == 8);
    assert(iter.count() == 8);

    static_assert(
      cuda::std::is_same_v<decltype(iter - cuda::std::default_sentinel), cuda::std::iter_difference_t<int*>>);
    static_assert(
      cuda::std::is_same_v<decltype(cuda::std::default_sentinel - iter), cuda::std::iter_difference_t<int*>>);
  }
  {
    const cuda::std::counted_iterator iter(forward_iterator<int*>{buffer}, 8);
    assert(iter - cuda::std::default_sentinel == -8);
    assert(cuda::std::default_sentinel - iter == 8);
    assert(iter.count() == 8);

    static_assert(
      cuda::std::is_same_v<decltype(iter - cuda::std::default_sentinel), cuda::std::iter_difference_t<int*>>);
    static_assert(
      cuda::std::is_same_v<decltype(cuda::std::default_sentinel - iter), cuda::std::iter_difference_t<int*>>);
  }
  {
    cuda::std::counted_iterator iter(cpp20_input_iterator<int*>{buffer}, 8);
    assert(iter - cuda::std::default_sentinel == -8);
    assert(cuda::std::default_sentinel - iter == 8);
    assert(iter.count() == 8);

    static_assert(
      cuda::std::is_same_v<decltype(iter - cuda::std::default_sentinel), cuda::std::iter_difference_t<int*>>);
    static_assert(
      cuda::std::is_same_v<decltype(cuda::std::default_sentinel - iter), cuda::std::iter_difference_t<int*>>);
  }
  {
    const cuda::std::counted_iterator iter(cpp20_input_iterator<int*>{buffer}, 8);
    assert(iter - cuda::std::default_sentinel == -8);
    assert(cuda::std::default_sentinel - iter == 8);
    assert(iter.count() == 8);

    static_assert(
      cuda::std::is_same_v<decltype(iter - cuda::std::default_sentinel), cuda::std::iter_difference_t<int*>>);
    static_assert(
      cuda::std::is_same_v<decltype(cuda::std::default_sentinel - iter), cuda::std::iter_difference_t<int*>>);
  }

  return true;
}

int main(int, char**)
{
  test();
  static_assert(test());

  return 0;
}
