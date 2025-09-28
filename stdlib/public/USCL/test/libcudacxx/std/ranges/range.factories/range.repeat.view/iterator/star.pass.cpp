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

// constexpr const W & operator*() const noexcept;

#include <uscl/std/cassert>
#include <uscl/std/concepts>
#include <uscl/std/ranges>

#include "test_macros.h"

__host__ __device__ constexpr bool test()
{
  // unbound
  {
    cuda::std::ranges::repeat_view<int> v(31);
    auto iter = v.begin();

    const int& val = *iter;
    for (int i = 0; i < 100; ++i, ++iter)
    {
      assert(*iter == 31);
      assert(&*iter == &val);
    }

    static_assert(noexcept(*iter));
    static_assert(cuda::std::same_as<decltype(*iter), const int&>);
  }

  // bound && one element
  {
    cuda::std::ranges::repeat_view<int, int> v(31, 1);
    auto iter = v.begin();
    assert(*iter == 31);
    static_assert(noexcept(*iter));
    static_assert(cuda::std::same_as<decltype(*iter), const int&>);
  }

  // bound && several elements
  {
    cuda::std::ranges::repeat_view<int, int> v(31, 100);
    auto iter = v.begin();

    const int& val = *iter;
    for (int i = 0; i < 100; ++i, ++iter)
    {
      assert(*iter == 31);
      assert(&*iter == &val);
    }
  }

#if TEST_STD_VER >= 2023 // requires lifetime expansion of the temporary view
  // bound && foreach
  {
    for (const auto& val : cuda::std::views::repeat(31, 100))
    {
      assert(val == 31);
    }
  }
#endif // TEST_STD_VER >= 2023

  return true;
}

int main(int, char**)
{
  test();
  static_assert(test());

  return 0;
}
