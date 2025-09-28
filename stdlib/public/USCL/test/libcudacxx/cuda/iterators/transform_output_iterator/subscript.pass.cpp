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

// constexpr auto operator[](difference_type n) const;

#include <uscl/iterator>
#include <uscl/std/cassert>

#include "test_macros.h"
#include "types.h"

template <class Fn>
__host__ __device__ constexpr void test()
{
  int buffer[8] = {0, 1, 2, 3, 4, 5, 6, 7};
  Fn func{};

  {
    cuda::transform_output_iterator iter{buffer, func};
    for (int i = 0; i < 8; ++i)
    {
      iter[i] = i;
      assert(buffer[i] == i + 1);
    }
    static_assert(noexcept(iter[0]));
    static_assert(noexcept(iter[0] = 2) == !cuda::std::is_same_v<Fn, PlusOneMayThrow>);
    static_assert(!cuda::std::is_same_v<decltype(*iter), int>);
  }

  {
    const cuda::transform_output_iterator iter{buffer, func};
    iter[0] = 2;
    assert(buffer[0] == 2 + 1);
    static_assert(noexcept(iter[0]));
    static_assert(noexcept(iter[0] = 2) == !cuda::std::is_same_v<Fn, PlusOneMayThrow>);
    static_assert(!cuda::std::is_same_v<decltype(*iter), int>);
  }
}

__host__ __device__ constexpr bool test()
{
  test<PlusOne>();
  test<PlusOneMutable>();
  test<PlusOneMayThrow>();
  NV_IF_ELSE_TARGET(NV_IS_HOST, (test<PlusOneHost>();), (test<PlusOneDevice>();))

  return true;
}

int main(int, char**)
{
  test();
  static_assert(test(), "");

  return 0;
}
