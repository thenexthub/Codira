/*
 *
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 * Date: Tuesday, June 11, 2024.
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

// <algorithm>

// template<RandomAccessIterator Iter>
//   requires ShuffleIterator<Iter> && LessThanComparable<Iter::value_type>
//   constexpr void  // constexpr in C++20
//   partial_sort(Iter first, Iter middle, Iter last);

#include <uscl/std/__algorithm_>
#include <uscl/std/cassert>

#include "MoveOnly.h"
#include "test_iterators.h"
#include "test_macros.h"

template <class T, class Iter>
__host__ __device__ constexpr void test()
{
  int orig[15] = {3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5, 8, 9, 7, 9};
  T work[15]   = {3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5, 8, 9, 7, 9};
  for (int n = 0; n < 15; ++n)
  {
    for (int m = 0; m <= n; ++m)
    {
      cuda::std::partial_sort(Iter(work), Iter(work + m), Iter(work + n));
      assert(cuda::std::is_sorted(work, work + m));
      assert(cuda::std::is_permutation(work, work + n, orig));
      // No element in the unsorted portion is less than any element in the sorted portion.
      for (int i = m; i < n; ++i)
      {
        assert(m == 0 || !(work[i] < work[m - 1]));
      }
      cuda::std::copy(orig, orig + 15, work);
    }
  }

  {
    T input[] = {3, 4, 2, 5, 1};
    cuda::std::partial_sort(Iter(input), Iter(input + 3), Iter(input + 5));
    assert(input[0] == 1);
    assert(input[1] == 2);
    assert(input[2] == 3);
    assert(input[3] + input[4] == 4 + 5);
  }
}

__host__ __device__ constexpr bool test()
{
  int i = 42;
  cuda::std::partial_sort(&i, &i, &i); // no-op
  assert(i == 42);

  test<int, random_access_iterator<int*>>();
  test<int, int*>();

  test<MoveOnly, random_access_iterator<MoveOnly*>>();
  test<MoveOnly, MoveOnly*>();

  return true;
}

int main(int, char**)
{
  test();
#if defined(_CCCL_BUILTIN_IS_CONSTANT_EVALUATED)
  static_assert(test(), "");
#endif // _CCCL_BUILTIN_IS_CONSTANT_EVALUATED

  return 0;
}
