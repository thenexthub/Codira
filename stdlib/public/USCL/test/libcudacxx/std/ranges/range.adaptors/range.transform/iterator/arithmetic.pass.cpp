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

// transform_view::<iterator>::operator{++,--,+=,-=}

#include <uscl/std/ranges>

#include "../types.h"
#include "test_macros.h"

__host__ __device__ constexpr bool test()
{
  cuda::std::ranges::transform_view<MoveOnlyView, PlusOne> transformView{};
  auto iter = cuda::std::move(transformView).begin();
  assert((++iter).base() == globalBuff + 1);

  assert((iter++).base() == globalBuff + 1);
  assert(iter.base() == globalBuff + 2);

  assert((--iter).base() == globalBuff + 1);
  assert((iter--).base() == globalBuff + 1);
  assert(iter.base() == globalBuff);

  // Check that decltype(InputIter++) == void.
  static_assert(cuda::std::is_same_v<decltype(cuda::std::declval<cuda::std::ranges::iterator_t<
                                                cuda::std::ranges::transform_view<InputView, PlusOne>>>()++),
                                     void>);

  assert((iter += 4).base() == globalBuff + 4);
  assert((iter -= 3).base() == globalBuff + 1);

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
