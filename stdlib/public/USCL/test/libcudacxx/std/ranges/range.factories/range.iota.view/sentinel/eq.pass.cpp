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

// friend constexpr bool operator==(const iterator& x, const sentinel& y);

#include <uscl/std/cassert>
#include <uscl/std/ranges>

#include "../types.h"
#include "test_macros.h"

__host__ __device__ constexpr bool test()
{
  {
    const cuda::std::ranges::iota_view<int, IntComparableWith<int>> io(0, IntComparableWith<int>(10));
    auto iter = io.begin();
    auto sent = io.end();
    assert(iter != sent);
    assert(iter + 10 == sent);
  }
  {
    cuda::std::ranges::iota_view<int, IntComparableWith<int>> io(0, IntComparableWith<int>(10));
    auto iter = io.begin();
    auto sent = io.end();
    assert(iter != sent);
    assert(iter + 10 == sent);
  }
  {
    const cuda::std::ranges::iota_view io(SomeInt(0), IntComparableWith(SomeInt(10)));
    auto iter = io.begin();
    auto sent = io.end();
    assert(iter != sent);
    assert(iter + 10 == sent);
  }
  {
    cuda::std::ranges::iota_view io(SomeInt(0), IntComparableWith(SomeInt(10)));
    auto iter = io.begin();
    auto sent = io.end();
    assert(iter != sent);
    assert(iter + 10 == sent);
  }

  return true;
}

int main(int, char**)
{
  test();
  static_assert(test(), "");

  return 0;
}
