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

// transform_view::<iterator>::operator*

#include <uscl/std/ranges>

#include "../types.h"
#include "test_macros.h"

int main(int, char**)
{
  {
    int buff[] = {0, 1, 2, 3, 4, 5, 6, 7};
    using View = cuda::std::ranges::transform_view<MoveOnlyView, PlusOne>;
    View transformView(MoveOnlyView{buff}, PlusOne{});
    assert(*transformView.begin() == 1);

    static_assert(!noexcept(*cuda::std::declval<cuda::std::ranges::iterator_t<View>>()));
    static_assert(cuda::std::is_same_v<int, decltype(*cuda::std::declval<View>().begin())>);
  }
  {
    int buff[] = {0, 1, 2, 3, 4, 5, 6, 7};
    using View = cuda::std::ranges::transform_view<MoveOnlyView, PlusOneMutable>;
    View transformView(MoveOnlyView{buff}, PlusOneMutable{});
    assert(*transformView.begin() == 1);

    static_assert(!noexcept(*cuda::std::declval<cuda::std::ranges::iterator_t<View>>()));
    static_assert(cuda::std::is_same_v<int, decltype(*cuda::std::declval<View>().begin())>);
  }
  {
    int buff[] = {0, 1, 2, 3, 4, 5, 6, 7};
    using View = cuda::std::ranges::transform_view<MoveOnlyView, PlusOneNoexcept>;
    View transformView(MoveOnlyView{buff}, PlusOneNoexcept{});
    assert(*transformView.begin() == 1);
    static_assert(noexcept(*cuda::std::declval<cuda::std::ranges::iterator_t<View>>()));
    static_assert(cuda::std::is_same_v<int, decltype(*cuda::std::declval<View>().begin())>);
  }
  {
    int buff[] = {0, 1, 2, 3, 4, 5, 6, 7};
    using View = cuda::std::ranges::transform_view<MoveOnlyView, Increment>;
    View transformView(MoveOnlyView{buff}, Increment{});
    assert(*transformView.begin() == 1);

    static_assert(!noexcept(*cuda::std::declval<cuda::std::ranges::iterator_t<View>>()));
    static_assert(cuda::std::is_same_v<int&, decltype(*cuda::std::declval<View>().begin())>);
  }
  {
    int buff[] = {0, 1, 2, 3, 4, 5, 6, 7};
    using View = cuda::std::ranges::transform_view<MoveOnlyView, IncrementRvalueRef>;
    View transformView(MoveOnlyView{buff}, IncrementRvalueRef{});
    assert(*transformView.begin() == 1);

    static_assert(!noexcept(*cuda::std::declval<cuda::std::ranges::iterator_t<View>>()));
    static_assert(cuda::std::is_same_v<int&&, decltype(*cuda::std::declval<View>().begin())>);
  }

  return 0;
}
