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

// constexpr counted_iterator operator-(iter_difference_t<I> n) const
//   requires random_access_iterator<I>;

#include <uscl/std/iterator>

#include "test_iterators.h"
#include "test_macros.h"

template <class Iter>
_CCCL_CONCEPT MinusEnabled = _CCCL_REQUIRES_EXPR((Iter), Iter& iter)((iter - 1));

__host__ __device__ constexpr bool test()
{
  int buffer[8] = {1, 2, 3, 4, 5, 6, 7, 8};

  {
    using Counted = cuda::std::counted_iterator<random_access_iterator<int*>>;
    Counted iter(random_access_iterator<int*>{buffer + 2}, 6);
    assert(iter - 2 == Counted(random_access_iterator<int*>{buffer}, 8));
    assert(iter - 0 == Counted(random_access_iterator<int*>{buffer + 2}, 6));
    assert(iter.count() == 6);

    static_assert(cuda::std::is_same_v<decltype(iter - 2), Counted>);
  }
  {
    using Counted = cuda::std::counted_iterator<random_access_iterator<int*>>;
    const Counted iter(random_access_iterator<int*>{buffer + 2}, 6);
    assert(iter - 2 == Counted(random_access_iterator<int*>{buffer}, 8));
    assert(iter - 0 == Counted(random_access_iterator<int*>{buffer + 2}, 6));
    assert(iter.count() == 6);

    static_assert(cuda::std::is_same_v<decltype(iter - 2), Counted>);
  }
  {
    using Counted = cuda::std::counted_iterator<contiguous_iterator<int*>>;
    Counted iter(contiguous_iterator<int*>{buffer + 2}, 6);
    assert(iter - 2 == Counted(contiguous_iterator<int*>{buffer}, 8));
    assert(iter - 0 == Counted(contiguous_iterator<int*>{buffer + 2}, 6));
    assert(iter.count() == 6);

    static_assert(cuda::std::is_same_v<decltype(iter - 2), Counted>);
  }
  {
    using Counted = cuda::std::counted_iterator<contiguous_iterator<int*>>;
    const Counted iter(contiguous_iterator<int*>{buffer + 2}, 6);
    assert(iter - 2 == Counted(contiguous_iterator<int*>{buffer}, 8));
    assert(iter - 0 == Counted(contiguous_iterator<int*>{buffer + 2}, 6));
    assert(iter.count() == 6);

    static_assert(cuda::std::is_same_v<decltype(iter - 2), Counted>);
  }

  {
    static_assert(MinusEnabled<cuda::std::counted_iterator<random_access_iterator<int*>>>);
    static_assert(!MinusEnabled<cuda::std::counted_iterator<bidirectional_iterator<int*>>>);
  }

  return true;
}

int main(int, char**)
{
  test();
  static_assert(test());

  return 0;
}
