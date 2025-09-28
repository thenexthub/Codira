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

// constexpr auto size() requires sized_range<V>
// constexpr auto size() const requires sized_range<const V>

#include <uscl/std/ranges>

#include "test_macros.h"
#include "types.h"

template <class T>
_CCCL_CONCEPT SizeInvocable = _CCCL_REQUIRES_EXPR((T), T t)(((void) t.size()));

__host__ __device__ constexpr bool test()
{
  {
    cuda::std::ranges::transform_view transformView(MoveOnlyView{}, PlusOne{});
    assert(transformView.size() == 8);
  }

  {
    const cuda::std::ranges::transform_view transformView(MoveOnlyView{globalBuff, 4}, PlusOne{});
    assert(transformView.size() == 4);
  }

  static_assert(!SizeInvocable<cuda::std::ranges::transform_view<ForwardView, PlusOne>>);

  static_assert(SizeInvocable<cuda::std::ranges::transform_view<SizedSentinelNotConstView, PlusOne>>);
  static_assert(!SizeInvocable<const cuda::std::ranges::transform_view<SizedSentinelNotConstView, PlusOne>>);

  return true;
}

int main(int, char**)
{
  test();
  static_assert(test(), "");

  return 0;
}
