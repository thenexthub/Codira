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

// constexpr V base() const& requires copy_constructible<V>
// constexpr V base() &&

#include <uscl/std/ranges>

#include "test_macros.h"
#include "types.h"

__host__ __device__ constexpr bool test()
{
  {
    cuda::std::ranges::transform_view<MoveOnlyView, PlusOne> transformView{};
    MoveOnlyView base = cuda::std::move(transformView).base();
    static_assert(cuda::std::is_same_v<MoveOnlyView, decltype(cuda::std::move(transformView).base())>);
    assert(cuda::std::ranges::begin(base) == globalBuff);
  }

  {
    cuda::std::ranges::transform_view<CopyableView, PlusOne> transformView{};
    CopyableView base1 = transformView.base();
    static_assert(cuda::std::is_same_v<CopyableView, decltype(transformView.base())>);
    assert(cuda::std::ranges::begin(base1) == globalBuff);

    CopyableView base2 = cuda::std::move(transformView).base();
    static_assert(cuda::std::is_same_v<CopyableView, decltype(cuda::std::move(transformView).base())>);
    assert(cuda::std::ranges::begin(base2) == globalBuff);
  }

  {
    const cuda::std::ranges::transform_view<CopyableView, PlusOne> transformView{};
    const CopyableView base1 = transformView.base();
    static_assert(cuda::std::is_same_v<CopyableView, decltype(transformView.base())>);
    assert(cuda::std::ranges::begin(base1) == globalBuff);

    const CopyableView base2 = cuda::std::move(transformView).base();
    static_assert(cuda::std::is_same_v<CopyableView, decltype(cuda::std::move(transformView).base())>);
    assert(cuda::std::ranges::begin(base2) == globalBuff);
  }

  return true;
}

int main(int, char**)
{
  test();
  static_assert(test(), "");

  return 0;
}
