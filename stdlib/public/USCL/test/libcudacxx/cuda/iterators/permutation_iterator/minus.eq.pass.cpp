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

// constexpr permutation_iterator& operator-=(iter_difference_t<I> n);

#include <uscl/iterator>

#include "test_iterators.h"
#include "test_macros.h"

__host__ __device__ constexpr bool test()
{
  int buffer[] = {1, 2, 3, 4, 5, 6, 7, 8};

  {
    using indexIter            = random_access_iterator<const int*>;
    using permutation_iterator = cuda::permutation_iterator<int*, indexIter>;
    const int offset[]         = {4, 3, 2, 5};
    const int diff             = 2;

    permutation_iterator iter(buffer, indexIter{offset + 3});
    assert((iter -= diff) == permutation_iterator(buffer, indexIter{offset + 1}));
    assert((iter -= 0) == permutation_iterator(buffer, indexIter{offset + 1}));
    assert(iter.index() == offset[1]);

    static_assert(cuda::std::is_same_v<decltype(iter -= 2), permutation_iterator&>);

    // The test iterators are not noexcept
    static_assert(!noexcept(iter -= diff));
    static_assert(noexcept(cuda::permutation_iterator<int*, int*>() -= diff));
  }

  return true;
}

int main(int, char**)
{
  test();
  static_assert(test());

  return 0;
}
