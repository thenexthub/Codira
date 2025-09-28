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

// constexpr iterator(Fn);
// constexpr explicit iterator(Fn, Integer);

#include <uscl/iterator>
#include <uscl/std/cassert>

#include "test_iterators.h"
#include "test_macros.h"
#include "types.h"

template <class Fn>
__host__ __device__ constexpr bool test()
{
  int buffer[8] = {0, 1, 2, 3, 4, 5, 6, 7};
  Fn func{};

  { // CTAD
    cuda::transform_output_iterator iter{random_access_iterator{buffer + 2}, func};
    assert(base(iter.base()) == buffer + 2);
    *iter = 3;
    assert(buffer[2] == 3 + 1);
    buffer[2] = 2;
#if !TEST_COMPILER(GCC, <, 9) && !TEST_COMPILER(MSVC)
    // The test iterators are not `is_nothrow_move_constructible`
    static_assert(!noexcept(cuda::transform_output_iterator{random_access_iterator{buffer + 2}, func}));
#endif // !TEST_COMPILER(GCC, <, 9) && !TEST_COMPILER(MSVC)
    static_assert(
      cuda::std::is_same_v<decltype(iter), cuda::transform_output_iterator<random_access_iterator<int*>, Fn>>);
  }

  { // CTAD
    cuda::transform_output_iterator iter{buffer + 2, func};
    assert(iter.base() == buffer + 2);
    *iter = 3;
    assert(buffer[2] == 3 + 1);
    buffer[2] = 2;
    static_assert(noexcept(cuda::transform_output_iterator{buffer + 2, func}));
    static_assert(cuda::std::is_same_v<decltype(iter), cuda::transform_output_iterator<int*, Fn>>);
  }

  {
    cuda::transform_output_iterator<random_access_iterator<int*>, Fn> iter{random_access_iterator{buffer + 2}, func};
    assert(base(iter.base()) == buffer + 2);
    *iter = 3;
    assert(buffer[2] == 3 + 1);
    buffer[2] = 2;
#if !TEST_COMPILER(GCC, <, 9) && !TEST_COMPILER(MSVC)
    // The test iterators are not `is_nothrow_move_constructible`
    static_assert(!noexcept(
      cuda::transform_output_iterator<random_access_iterator<int*>, Fn>{random_access_iterator{buffer + 2}, func}));
#endif // !TEST_COMPILER(GCC, <, 9) && !TEST_COMPILER(MSVC)
  }

  {
    cuda::transform_output_iterator<int*, Fn> iter{buffer + 2, func};
    assert(iter.base() == buffer + 2);
    *iter = 3;
    assert(buffer[2] == 3 + 1);
    buffer[2] = 2;
    static_assert(noexcept(cuda::transform_output_iterator<int*, Fn>{buffer + 2, func}));
  }

  return true;
}

__host__ __device__ constexpr bool test()
{
  test<PlusOne>();
  NV_IF_ELSE_TARGET(NV_IS_HOST, (test<PlusOneHost>();), (test<PlusOneDevice>();))

  return true;
}

int main(int, char**)
{
  test();
  static_assert(test(), "");

  return 0;
}
