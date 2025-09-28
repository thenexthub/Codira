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

// constexpr auto operator->() const noexcept
//   requires contiguous_iterator<I>;

#include <uscl/std/iterator>
#include <uscl/std/utility>

#include "test_iterators.h"
#include "test_macros.h"

template <class Iter>
_CCCL_CONCEPT ArrowEnabled = _CCCL_REQUIRES_EXPR((Iter), Iter& iter)(unused(iter.operator->()));

__host__ __device__ constexpr bool test()
{
  int buffer[8] = {1, 2, 3, 4, 5, 6, 7, 8};

  {
    cuda::std::counted_iterator iter(contiguous_iterator<int*>{buffer}, 8);
    for (int i = 0; i < 8; ++i, ++iter)
    {
      assert(iter.operator->() == buffer + i);
    }

    static_assert(noexcept(iter.operator->()));
  }
  {
    const cuda::std::counted_iterator iter(contiguous_iterator<int*>{buffer}, 8);
    assert(iter.operator->() == buffer);

    static_assert(noexcept(iter.operator->()));
  }

  {
    static_assert(ArrowEnabled<cuda::std::counted_iterator<contiguous_iterator<int*>>>);
    static_assert(!ArrowEnabled<cuda::std::counted_iterator<random_access_iterator<int*>>>);
  }

  return true;
}

int main(int, char**)
{
  test();
  static_assert(test());

  return 0;
}
