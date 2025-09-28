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

// template<class W, class Bound>
//     requires (!is-integer-like<W> || !is-integer-like<Bound> ||
//               (is-signed-integer-like<W> == is-signed-integer-like<Bound>))
//     iota_view(W, Bound) -> iota_view<W, Bound>;

#include <uscl/std/cassert>
#include <uscl/std/concepts>
#include <uscl/std/ranges>

#include "test_macros.h"
#include "types.h"

template <class T, class U>
_CCCL_CONCEPT CanDeduce = _CCCL_REQUIRES_EXPR((T, U), const T& t, const U& u)(cuda::std::ranges::iota_view(t, u));

__host__ __device__ void test()
{
  static_assert(
    cuda::std::same_as<decltype(cuda::std::ranges::iota_view(0, 0)), cuda::std::ranges::iota_view<int, int>>);

  static_assert(cuda::std::same_as<decltype(cuda::std::ranges::iota_view(0)),
                                   cuda::std::ranges::iota_view<int, cuda::std::unreachable_sentinel_t>>);

  static_assert(cuda::std::same_as<decltype(cuda::std::ranges::iota_view(0, cuda::std::unreachable_sentinel)),
                                   cuda::std::ranges::iota_view<int, cuda::std::unreachable_sentinel_t>>);

  static_assert(cuda::std::same_as<decltype(cuda::std::ranges::iota_view(0, IntComparableWith(0))),
                                   cuda::std::ranges::iota_view<int, IntComparableWith<int>>>);

  static_assert(CanDeduce<int, int>);
  static_assert(!CanDeduce<int, unsigned>);
  static_assert(!CanDeduce<unsigned, int>);
}

int main(int, char**)
{
  return 0;
}
