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
//   push_heap(Iter first, Iter last);

#include <uscl/std/__algorithm_>
#include <uscl/std/cassert>
#include <uscl/std/functional>

#include "MoveOnly.h"
#include "test_iterators.h"
#include "test_macros.h"

template <class T, class Iter>
__host__ __device__ constexpr void test()
{
  T orig[15] = {3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5, 8, 9, 7, 9};
  T work[15] = {3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5, 8, 9, 7, 9};
  for (int i = 1; i < 15; ++i)
  {
    cuda::std::push_heap(Iter(work), Iter(work + i));
    assert(cuda::std::is_permutation(work, work + i, orig));
    assert(cuda::std::is_heap(work, work + i));
  }

  {
    T input[] = {1, 3, 2, 5, 4};
    cuda::std::push_heap(Iter(input), Iter(input + 1));
    assert(input[0] == 1);
    cuda::std::push_heap(Iter(input), Iter(input + 2));
    assert(input[0] == 3);
    cuda::std::push_heap(Iter(input), Iter(input + 3));
    assert(input[0] == 3);
    cuda::std::push_heap(Iter(input), Iter(input + 4));
    assert(input[0] == 5);
    cuda::std::push_heap(Iter(input), Iter(input + 5));
    assert(input[0] == 5);
    assert(cuda::std::is_heap(input, input + 5));
  }
}

__host__ __device__ constexpr bool test()
{
  test<int, random_access_iterator<int*>>();
  test<int, int*>();
  test<MoveOnly, random_access_iterator<MoveOnly*>>();
  test<MoveOnly, MoveOnly*>();

  return true;
}

int main(int, char**)
{
  test();
  static_assert(test(), "");

  return 0;
}
