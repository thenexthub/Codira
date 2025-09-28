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

// friend constexpr iter_difference_t operator-(const permutation_iterator& x, const permutation_iterator& y);

#include <uscl/iterator>

#include "test_iterators.h"
#include "test_macros.h"

__host__ __device__ constexpr bool test()
{
  using indexIter = random_access_iterator<const int*>;
  int buffer[]    = {1, 2, 3, 4, 5, 6, 7, 8};

  { // Iterators with same base
    const int offset[] = {2, 4};
    cuda::permutation_iterator iter1(buffer, indexIter{offset});
    cuda::permutation_iterator iter2(buffer, indexIter{offset + 1});
    assert(iter1 - iter2 == -1);
    assert(iter2 - iter1 == 1);
    assert(iter1.index() == offset[0]);
    assert(iter2.index() == offset[1]);

    static_assert(cuda::std::is_same_v<decltype(iter1 - iter2), cuda::std::iter_difference_t<int*>>);
  }

  { // const iterators with same base
    const int offset[] = {2, 4};
    const cuda::permutation_iterator iter1(buffer, indexIter{offset});
    const cuda::permutation_iterator iter2(buffer, indexIter{offset + 1});
    assert(iter1 - iter2 == -1);
    assert(iter2 - iter1 == 1);
    assert(iter1.index() == offset[0]);
    assert(iter2.index() == offset[1]);

    static_assert(cuda::std::is_same_v<decltype(iter1 - iter2), cuda::std::iter_difference_t<int*>>);
  }

  return true;
}

int main(int, char**)
{
  test();
  static_assert(test());

  return 0;
}
