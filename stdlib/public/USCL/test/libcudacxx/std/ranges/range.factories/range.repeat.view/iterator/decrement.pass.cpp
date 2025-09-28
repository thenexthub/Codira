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

// constexpr iterator& operator--();
// constexpr iterator operator--(int);

#include <uscl/std/cassert>
#include <uscl/std/ranges>

#include "test_macros.h"

__host__ __device__ constexpr bool test()
{
  using Iter = cuda::std::ranges::iterator_t<cuda::std::ranges::repeat_view<int>>;
  cuda::std::ranges::repeat_view<int> rv(10);
  auto iter = rv.begin() + 10;

  assert(iter-- == rv.begin() + 10);
  assert(--iter == rv.begin() + 8);

  static_assert(cuda::std::same_as<decltype(iter--), Iter>);
  static_assert(cuda::std::same_as<decltype(--iter), Iter&>);

  return true;
}

int main(int, char**)
{
  test();
  static_assert(test());

  return 0;
}
