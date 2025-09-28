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

// constexpr reference operator[](difference_type n) noexcept;
// constexpr const_reference operator[](difference_type n) const noexcept;

#include <uscl/iterator>
#include <uscl/std/cassert>

#include "test_macros.h"
#include "types.h"

template <class Stride>
__host__ __device__ constexpr void test(Stride stride)
{
  int buffer[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  {
    cuda::strided_iterator iter{buffer, stride};
    assert(iter[2] == *(buffer + 2 * iter.stride()));
    assert(cuda::std::addressof(iter[2]) == buffer + 2 * iter.stride());

    ++iter;
    assert(iter[2] == *(buffer + 3 * iter.stride()));
    assert(cuda::std::addressof(iter[2]) == buffer + 3 * iter.stride());
    static_assert(noexcept(iter[2]));
    static_assert(cuda::std::is_same_v<decltype(iter[2]), cuda::std::iter_reference_t<int*>>);
  }

  {
    const cuda::strided_iterator citer{buffer, stride};
    assert(citer[2] == *(buffer + 2 * citer.stride()));
    assert(cuda::std::addressof(citer[2]) == buffer + 2 * citer.stride());

    static_assert(noexcept(citer[2]));
    static_assert(cuda::std::is_same_v<decltype(citer[2]), cuda::std::iter_reference_t<int*>>);
  }
}

__host__ __device__ constexpr bool test()
{
  test(2);
  test(Stride<2>{});

  return true;
}

int main(int, char**)
{
  test();
  // static_assert(test(), "");

  return 0;
}
