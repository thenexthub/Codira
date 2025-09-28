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

// friend constexpr decltype(auto) iter_move(const iterator& i)
//    noexcept(noexcept(invoke(i.parent_->fun_, *i.current_)))

#include <uscl/std/ranges>

#include "../types.h"
#include "test_macros.h"

__host__ __device__ constexpr bool test()
{
  int buff[8] = {0, 1, 2, 3, 4, 5, 6, 7};

  {
    cuda::std::ranges::transform_view transformView(MoveOnlyView{buff}, PlusOneMutable{});
    auto iter = transformView.begin();

    static_assert(!noexcept(cuda::std::ranges::iter_move(iter)));

    assert(cuda::std::ranges::iter_move(iter) == 1);
    assert(cuda::std::ranges::iter_move(iter + 2) == 3);

    static_assert(cuda::std::is_same_v<int, decltype(cuda::std::ranges::iter_move(iter))>);
    static_assert(cuda::std::is_same_v<int, decltype(cuda::std::ranges::iter_move(cuda::std::move(iter)))>);
  }

  {
    static_assert(noexcept(cuda::std::ranges::iter_move(
      cuda::std::declval<
        cuda::std::ranges::iterator_t<cuda::std::ranges::transform_view<MoveOnlyView, PlusOneNoexcept>>&>())));

    static_assert(!noexcept(cuda::std::ranges::iter_move(
      cuda::std::declval<
        cuda::std::ranges::iterator_t<cuda::std::ranges::transform_view<MoveOnlyView, PlusOneMutable>>&>())));
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
