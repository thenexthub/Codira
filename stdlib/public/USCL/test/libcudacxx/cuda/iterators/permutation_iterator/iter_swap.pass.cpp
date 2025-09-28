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

// template<indirectly_swappable<I> I2>
//   friend constexpr void
//     iter_swap(const permutation_iterator& x, const permutation_iterator<I2>& y)
//       noexcept(noexcept(ranges::iter_swap(x.current, y.current)));

#include <uscl/iterator>

#include "test_iterators.h"
#include "test_macros.h"

__host__ __device__ constexpr bool test()
{
  using baseIter      = random_access_iterator<int*>;
  using indexIter     = random_access_iterator<const int*>;
  int buffer[]        = {1, 2, 3, 4, 5, 6, 7, 8};
  const int offset1[] = {2};
  const int offset2[] = {4};
  auto iter1          = cuda::permutation_iterator(baseIter{buffer}, indexIter{offset1});
  auto iter2          = cuda::permutation_iterator(baseIter{buffer}, indexIter{offset2});

  assert(*iter1 == 3);
  assert(*iter2 == 5);
  cuda::std::ranges::iter_swap(iter1, iter2);
  assert(*iter1 == 5);
  assert(*iter2 == 3);
  cuda::std::ranges::iter_swap(iter1, iter2);
  assert(*iter1 == 3);
  assert(*iter2 == 5);

  // The test iterators are not noexcept
  static_assert(!noexcept(cuda::std::ranges::iter_swap(iter1, iter2)));
  static_assert(noexcept(
    cuda::std::ranges::iter_swap(cuda::permutation_iterator<int*, int*>(), cuda::permutation_iterator<int*, int*>())));

  return true;
}

int main(int, char**)
{
  test();
  static_assert(test());

  return 0;
}
