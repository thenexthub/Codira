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

// template<BidirectionalIterator Iter>
//   requires ShuffleIterator<Iter>
//         && LessThanComparable<Iter::value_type>
//   constexpr bool  // constexpr in C++20
//   next_permutation(Iter first, Iter last);

#include <uscl/std/__algorithm_>
#include <uscl/std/cassert>

#include "test_iterators.h"
#include "test_macros.h"

__host__ __device__ constexpr int factorial(int x)
{
  int r = 1;
  for (; x; --x)
  {
    r *= x;
  }
  return r;
}

template <class Iter>
__host__ __device__ constexpr void test()
{
  int ia[]     = {1, 2, 3, 4, 5, 6};
  const int sa = sizeof(ia) / sizeof(ia[0]);
  int prev[sa] = {};
  for (int e = 0; e <= sa; ++e)
  {
    int count = 0;
    bool x    = false;
    do
    {
      cuda::std::copy(ia, ia + e, prev);
      x = cuda::std::next_permutation(Iter(ia), Iter(ia + e));
      if (e > 1)
      {
        if (x)
        {
          assert(cuda::std::lexicographical_compare(prev, prev + e, ia, ia + e));
        }
        else
        {
          assert(cuda::std::lexicographical_compare(ia, ia + e, prev, prev + e));
        }
      }
      ++count;
    } while (x);
    assert(count == factorial(e));
  }
}

__host__ __device__ constexpr bool test()
{
  test<bidirectional_iterator<int*>>();
  test<random_access_iterator<int*>>();
  test<int*>();

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
