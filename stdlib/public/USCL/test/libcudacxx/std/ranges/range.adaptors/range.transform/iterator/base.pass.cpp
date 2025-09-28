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

// transform_view::<iterator>::base

#include <uscl/std/ranges>

#include "../types.h"
#include "test_macros.h"

__host__ __device__ constexpr bool test()
{
  {
    using TransformView = cuda::std::ranges::transform_view<MoveOnlyView, PlusOneMutable>;
    TransformView tv{};
    auto it  = tv.begin();
    using It = decltype(it);
    static_assert(cuda::std::is_same_v<decltype(static_cast<It&>(it).base()), int* const&>);
    static_assert(cuda::std::is_same_v<decltype(static_cast<It&&>(it).base()), int*>);
    static_assert(cuda::std::is_same_v<decltype(static_cast<const It&>(it).base()), int* const&>);
    static_assert(cuda::std::is_same_v<decltype(static_cast<const It&&>(it).base()), int* const&>);
    static_assert(noexcept(it.base()));
    assert(base(it.base()) == globalBuff);
    assert(base(cuda::std::move(it).base()) == globalBuff);
  }
  {
    using TransformView = cuda::std::ranges::transform_view<InputView, PlusOneMutable>;
    TransformView tv{};
    auto it  = tv.begin();
    using It = decltype(it);
    static_assert(cuda::std::is_same_v<decltype(static_cast<It&>(it).base()), const cpp20_input_iterator<int*>&>);
    static_assert(cuda::std::is_same_v<decltype(static_cast<It&&>(it).base()), cpp20_input_iterator<int*>>);
    static_assert(cuda::std::is_same_v<decltype(static_cast<const It&>(it).base()), const cpp20_input_iterator<int*>&>);
    static_assert(
      cuda::std::is_same_v<decltype(static_cast<const It&&>(it).base()), const cpp20_input_iterator<int*>&>);
    static_assert(noexcept(it.base()));
    assert(base(it.base()) == globalBuff);
    assert(base(cuda::std::move(it).base()) == globalBuff);
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
