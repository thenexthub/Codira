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

// constexpr T* begin() noexcept;
// constexpr const T* begin() const noexcept;

#include <uscl/std/cassert>
#include <uscl/std/ranges>

#include "test_macros.h"

struct Empty
{};
struct BigType
{
  char buffer[64] = {10};
};

__host__ __device__ constexpr bool test()
{
  {
    auto sv = cuda::std::ranges::single_view<int>(42);
    assert(*sv.begin() == 42);

    static_assert(cuda::std::is_same_v<decltype(sv.begin()), int*>);
    static_assert(noexcept(sv.begin()));
  }
  {
    const auto sv = cuda::std::ranges::single_view<int>(42);
    assert(*sv.begin() == 42);

    static_assert(cuda::std::is_same_v<decltype(sv.begin()), const int*>);
    static_assert(noexcept(sv.begin()));
  }

  {
    auto sv = cuda::std::ranges::single_view<Empty>(Empty());
    assert(sv.begin() != nullptr);

    static_assert(cuda::std::is_same_v<decltype(sv.begin()), Empty*>);
  }
  {
    const auto sv = cuda::std::ranges::single_view<Empty>(Empty());
    assert(sv.begin() != nullptr);

    static_assert(cuda::std::is_same_v<decltype(sv.begin()), const Empty*>);
  }

  {
    auto sv = cuda::std::ranges::single_view<BigType>(BigType());
    assert(sv.begin()->buffer[0] == 10);

    static_assert(cuda::std::is_same_v<decltype(sv.begin()), BigType*>);
  }
  {
    const auto sv = cuda::std::ranges::single_view<BigType>(BigType());
    assert(sv.begin()->buffer[0] == 10);

    static_assert(cuda::std::is_same_v<decltype(sv.begin()), const BigType*>);
  }

  return true;
}

int main(int, char**)
{
  test();
#if defined(_CCCL_BUILTIN_ADDRESSOF)
  static_assert(test());
#endif // _CCCL_BUILTIN_ADDRESSOF

  return 0;
}
