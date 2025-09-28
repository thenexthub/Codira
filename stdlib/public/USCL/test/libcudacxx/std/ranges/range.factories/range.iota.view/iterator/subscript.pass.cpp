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

// constexpr W operator[](difference_type n) const
//   requires advanceable<W>;

#include <uscl/std/cassert>
#include <uscl/std/ranges>

#include "../types.h"
#include "test_macros.h"

template <class T>
__host__ __device__ constexpr void testType()
{
  {
    cuda::std::ranges::iota_view<T> io(T(0));
    auto iter = io.begin();
    for (int i = 0; i < 100; ++i)
    {
      assert(iter[i] == T(i));
    }
  }
  {
    cuda::std::ranges::iota_view<T> io(T(10));
    auto iter = io.begin();
    for (int i = 0; i < 100; ++i)
    {
      assert(iter[i] == T(i + 10));
    }
  }
  {
    const cuda::std::ranges::iota_view<T> io(T(0));
    auto iter = io.begin();
    for (int i = 0; i < 100; ++i)
    {
      assert(iter[i] == T(i));
    }
  }
  {
    const cuda::std::ranges::iota_view<T> io(T(10));
    auto iter = io.begin();
    for (int i = 0; i < 100; ++i)
    {
      assert(iter[i] == T(i + 10));
    }
  }
}

__host__ __device__ constexpr bool test()
{
  testType<SomeInt>();
  testType<signed long>();
  testType<unsigned long>();
  testType<int>();
  testType<unsigned>();
  testType<short>();
  testType<unsigned short>();

  return true;
}

int main(int, char**)
{
  test();
  static_assert(test(), "");

  return 0;
}
