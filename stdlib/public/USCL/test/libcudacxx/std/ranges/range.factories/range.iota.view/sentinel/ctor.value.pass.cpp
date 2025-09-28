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

// constexpr explicit sentinel(Bound bound);

#include <uscl/std/cassert>
#include <uscl/std/ranges>

#include "../types.h"
#include "test_macros.h"

__host__ __device__ constexpr bool test()
{
  {
    using Sent = cuda::std::ranges::sentinel_t<cuda::std::ranges::iota_view<int, IntSentinelWith<int>>>;
    using Iter = cuda::std::ranges::iterator_t<cuda::std::ranges::iota_view<int, IntSentinelWith<int>>>;
    auto sent  = Sent(IntSentinelWith<int>(42));
    assert(sent == Iter(42));
  }
  {
    using Sent = cuda::std::ranges::sentinel_t<cuda::std::ranges::iota_view<SomeInt, IntSentinelWith<SomeInt>>>;
    using Iter = cuda::std::ranges::iterator_t<cuda::std::ranges::iota_view<SomeInt, IntSentinelWith<SomeInt>>>;
    auto sent  = Sent(IntSentinelWith<SomeInt>(SomeInt(42)));
    assert(sent == Iter(SomeInt(42)));
  }
  {
    using Sent = cuda::std::ranges::sentinel_t<cuda::std::ranges::iota_view<SomeInt, IntSentinelWith<SomeInt>>>;
    static_assert(!cuda::std::is_convertible_v<Sent, IntSentinelWith<SomeInt>>);
    static_assert(cuda::std::is_constructible_v<Sent, IntSentinelWith<SomeInt>>);
  }

  return true;
}

int main(int, char**)
{
  test();
  static_assert(test(), "");

  return 0;
}
