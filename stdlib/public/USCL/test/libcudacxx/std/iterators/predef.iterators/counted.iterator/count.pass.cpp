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

// constexpr iter_difference_t<I> count() const noexcept;

#include <uscl/std/iterator>

#include "test_iterators.h"
#include "test_macros.h"

struct InputOrOutputArchetype
{
  using difference_type = int;

  int* ptr;

  __host__ __device__ constexpr int operator*()
  {
    return *ptr;
  }
  __host__ __device__ constexpr void operator++(int)
  {
    ++ptr;
  }
  __host__ __device__ constexpr InputOrOutputArchetype& operator++()
  {
    ++ptr;
    return *this;
  }
};

__host__ __device__ constexpr bool test()
{
  int buffer[8] = {1, 2, 3, 4, 5, 6, 7, 8};

  {
    cuda::std::counted_iterator iter(cpp20_input_iterator<int*>{buffer}, 8);
    for (int i = 8; i != 0; --i, ++iter)
    {
      assert(iter.count() == i);
    }

    static_assert(noexcept(iter.count()));
  }
  {
    cuda::std::counted_iterator iter(forward_iterator<int*>{buffer}, 8);
    for (int i = 8; i != 0; --i, ++iter)
    {
      assert(iter.count() == i);
    }

    static_assert(noexcept(iter.count()));
  }
  {
    cuda::std::counted_iterator iter(contiguous_iterator<int*>{buffer}, 8);
    for (int i = 8; i != 0; --i, ++iter)
    {
      assert(iter.count() == i);
    }
  }
  {
    cuda::std::counted_iterator iter(InputOrOutputArchetype{buffer + 2}, 6);
    assert(iter.count() == 6);
  }

  // Const tests.
  {
    const cuda::std::counted_iterator iter(cpp20_input_iterator<int*>{buffer}, 8);
    assert(iter.count() == 8);
  }
  {
    const cuda::std::counted_iterator iter(forward_iterator<int*>{buffer + 1}, 7);
    assert(iter.count() == 7);
  }
  {
    const cuda::std::counted_iterator iter(contiguous_iterator<int*>{buffer + 2}, 6);
    assert(iter.count() == 6);
  }
  {
    const cuda::std::counted_iterator iter(InputOrOutputArchetype{buffer + 2}, 6);
    assert(iter.count() == 6);
  }

  return true;
}

int main(int, char**)
{
  test();
  static_assert(test());

  return 0;
}
