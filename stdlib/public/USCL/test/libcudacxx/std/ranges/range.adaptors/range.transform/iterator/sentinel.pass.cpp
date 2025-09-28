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

// class transform_view::<sentinel>;

#include <uscl/std/ranges>

#include "../types.h"
#include "test_macros.h"

template <class T>
_CCCL_CONCEPT EndIsIter = _CCCL_REQUIRES_EXPR((T), T t)((++t.end()));

__host__ __device__ constexpr bool test()
{
  cuda::std::ranges::transform_view<SizedSentinelView, PlusOne> transformView1{};
  // Going to const and back.
  auto sent1 = transformView1.end();
  cuda::std::ranges::sentinel_t<const cuda::std::ranges::transform_view<SizedSentinelView, PlusOne>> sent2{sent1};
  cuda::std::ranges::sentinel_t<const cuda::std::ranges::transform_view<SizedSentinelView, PlusOne>> sent3{sent2};
  unused(sent3);

  static_assert(!EndIsIter<decltype(sent1)>);
  static_assert(!EndIsIter<decltype(sent2)>);
  assert(sent1.base() == globalBuff + 8);

  cuda::std::ranges::transform_view transformView2(SizedSentinelView{4}, PlusOne());
  auto sent4 = transformView2.end();
  auto iter  = transformView1.begin();
  {
    assert(iter != sent1);
    assert(iter != sent2);
    assert(iter != sent4);
  }

  {
    assert(iter + 8 == sent1);
    assert(iter + 8 == sent2);
    assert(iter + 4 == sent4);
  }

  {
    assert(sent1 - iter == 8);
    assert(sent4 - iter == 4);
    assert(iter - sent1 == -8);
    assert(iter - sent4 == -4);
  }

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
