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

// template<ForwardIterator Iter, class T>
//   constexpr Iter    // constexpr after c++17
//   lower_bound(Iter first, Iter last, const T& value);

#include <uscl/std/__algorithm_>
#include <uscl/std/cassert>
#include <uscl/std/cstddef>

#include "../cases.h"
#include "test_iterators.h"
#include "test_macros.h"

template <class Iter, class T>
__host__ __device__ constexpr void test(Iter first, Iter last, const T& value)
{
  Iter i = cuda::std::lower_bound(first, last, value);
  for (Iter j = first; j != i; ++j)
  {
    assert(*j < value);
  }
  for (Iter j = i; j != last; ++j)
  {
    assert(!(*j < value));
  }
}

template <class Iter>
__host__ __device__ constexpr void test()
{
  constexpr int M = 10;
  auto v          = get_data(M);
  for (int x = 0; x < M; ++x)
  {
    test(Iter(cuda::std::begin(v)), Iter(cuda::std::end(v)), x);
  }
}

__host__ __device__ constexpr bool test()
{
  int d[] = {0, 1, 2, 3};
  for (int* e = d; e < d + 4; ++e)
  {
    for (int x = -1; x <= 4; ++x)
    {
      test(d, e, x);
    }
  }

  test<forward_iterator<const int*>>();
  test<bidirectional_iterator<const int*>>();
  test<random_access_iterator<const int*>>();
  test<const int*>();

  return true;
}

int main(int, char**)
{
  test();
  static_assert(test(), "");

  return 0;
}
