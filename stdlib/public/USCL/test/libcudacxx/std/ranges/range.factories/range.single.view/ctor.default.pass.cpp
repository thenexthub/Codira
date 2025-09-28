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

// single_view() requires default_initializable<T> = default;

#include <uscl/std/cassert>
#include <uscl/std/ranges>

#include "test_macros.h"

struct BigType
{
  char buffer[64] = {10};
};

template <bool DefaultCtorEnabled>
struct IsDefaultConstructible
{};

template <>
struct IsDefaultConstructible<false>
{
  IsDefaultConstructible() = delete;
};

__host__ __device__ constexpr bool test()
{
  static_assert(cuda::std::default_initializable<cuda::std::ranges::single_view<IsDefaultConstructible<true>>>);
  static_assert(!cuda::std::default_initializable<cuda::std::ranges::single_view<IsDefaultConstructible<false>>>);

  {
    cuda::std::ranges::single_view<BigType> sv;
    assert(sv.data()->buffer[0] == 10);
    assert(sv.size() == 1);
  }
  {
    const cuda::std::ranges::single_view<BigType> sv;
    assert(sv.data()->buffer[0] == 10);
    assert(sv.size() == 1);
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
