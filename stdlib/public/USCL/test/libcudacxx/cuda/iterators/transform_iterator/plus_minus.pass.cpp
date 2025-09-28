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

// transform_iterator::operator{+,-}

#include <uscl/iterator>
#include <uscl/std/cassert>

#include "test_iterators.h"
#include "test_macros.h"
#include "types.h"

template <class Iter>
_CCCL_CONCEPT can_plus = _CCCL_REQUIRES_EXPR((Iter), Iter i)((i + 42), (42 + i));

template <class Iter>
_CCCL_CONCEPT can_minus = _CCCL_REQUIRES_EXPR((Iter), Iter i)((i - 42));

template <class Iter>
__host__ __device__ constexpr void test()
{
  if constexpr (cuda::std::random_access_iterator<Iter>)
  {
    int buffer[8] = {0, 1, 2, 3, 4, 5, 6, 7};

    cuda::transform_iterator iter1{Iter{buffer + 4}, PlusOne{}};
    cuda::transform_iterator iter2{Iter{buffer}, PlusOne{}};

    assert((iter1 + 1).base() == Iter{buffer + 5});
    assert((1 + iter1).base() == Iter{buffer + 5});
    assert((iter1 - 1).base() == Iter{buffer + 3});
    assert(iter1 - iter2 == 4);
    assert((iter1 + 2) - 2 == iter1);
    assert((iter1 - 2) + 2 == iter1);
  }
  else
  {
    static_assert(!can_plus<cuda::transform_iterator<Iter, PlusOne>>);
    static_assert(!can_minus<cuda::transform_iterator<Iter, PlusOne>>);
  }
}

__host__ __device__ constexpr bool test()
{
  test<cpp17_input_iterator<int*>>();
  test<random_access_iterator<int*>>();
  test<int*>();

  return true;
}

int main(int, char**)
{
  test();
  static_assert(test(), "");

  return 0;
}
