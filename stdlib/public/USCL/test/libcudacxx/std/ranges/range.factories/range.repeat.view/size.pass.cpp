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

// constexpr auto size() const requires (!same_as<Bound, unreachable_sentinel_t>);

#include <uscl/std/cassert>
#include <uscl/std/iterator>
#include <uscl/std/limits>
#include <uscl/std/ranges>

#include "test_macros.h"

template <class T>
_CCCL_CONCEPT has_size = _CCCL_REQUIRES_EXPR((T), T&& view)((cuda::std::forward<T>(view).size()));

static_assert(has_size<cuda::std::ranges::repeat_view<int, int>>);
static_assert(!has_size<cuda::std::ranges::repeat_view<int>>);
static_assert(!has_size<cuda::std::ranges::repeat_view<int, cuda::std::unreachable_sentinel_t>>);

__host__ __device__ constexpr bool test()
{
  {
    cuda::std::ranges::repeat_view<int, int> rv(10, 20);
    assert(rv.size() == 20);
  }

  {
    constexpr int int_max = cuda::std::numeric_limits<int>::max();
    cuda::std::ranges::repeat_view<int, int> rv(10, int_max);
    assert(rv.size() == int_max);
  }

  return true;
}

int main(int, char**)
{
  test();
  static_assert(test());

  return 0;
}
