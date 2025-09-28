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

#include <uscl/iterator>
#include <uscl/std/cassert>
#include <uscl/std/type_traits>

#include "test_iterators.h"
#include "test_macros.h"
#include "types.h"

__host__ __device__ constexpr bool test()
{
  int buffer[8] = {0, 1, 2, 3, 4, 5, 6, 7};

  {
    cuda::transform_iterator iter{buffer, PlusOne{}};
    static_assert(!noexcept(cuda::std::ranges::iter_move(iter)));

    assert(cuda::std::ranges::iter_move(iter) == 1);
    assert(cuda::std::ranges::iter_move(iter + 2) == 3);

    static_assert(cuda::std::is_same_v<int, decltype(cuda::std::ranges::iter_move(iter))>);
    static_assert(cuda::std::is_same_v<int, decltype(cuda::std::ranges::iter_move(cuda::std::move(iter)))>);
  }

  {
    [[maybe_unused]] cuda::transform_iterator iter_noexcept{buffer, PlusOneNoexcept{}};
    static_assert(noexcept(cuda::std::ranges::iter_move(iter_noexcept)));

    [[maybe_unused]] cuda::transform_iterator iter_not_noexcept{buffer, PlusOneMutable{}};
    static_assert(!noexcept(cuda::std::ranges::iter_move(iter_not_noexcept)));
  }

  return true;
}

int main(int, char**)
{
  test();
  static_assert(test(), "");

  return 0;
}
