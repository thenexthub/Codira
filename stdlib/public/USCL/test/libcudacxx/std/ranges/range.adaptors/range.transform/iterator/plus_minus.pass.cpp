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

// transform_view::<iterator>::operator{+,-}

#include <uscl/std/ranges>

#include "../types.h"
#include "test_macros.h"

__host__ __device__ constexpr bool test()
{
  cuda::std::ranges::transform_view<MoveOnlyView, PlusOneMutable> transformView1{};
  auto iter1 = cuda::std::move(transformView1).begin();
  cuda::std::ranges::transform_view<MoveOnlyView, PlusOneMutable> transformView2{};
  auto iter2 = cuda::std::move(transformView2).begin();
  iter1 += 4;
  assert((iter1 + 1).base() == globalBuff + 5);
  assert((1 + iter1).base() == globalBuff + 5);
  assert((iter1 - 1).base() == globalBuff + 3);
  assert(iter1 - iter2 == 4);
  assert((iter1 + 2) - 2 == iter1);
  assert((iter1 - 2) + 2 == iter1);

  unused(iter2);
  return true;
}

int main(int, char**)
{
  test();
#if defined(_CCCL_BUILTIN_ADDRESSOF)
  static_assert(test(), "");
#endif // _CCCL_BUILTIN_ADDRESSOF

  return 0;
}
