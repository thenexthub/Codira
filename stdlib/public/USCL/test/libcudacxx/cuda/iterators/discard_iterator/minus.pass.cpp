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

// constexpr discard_iterator operator-(iter_difference_t<I> n) const;
// friend constexpr iter_difference_t operator-(const discard_iterator& x, const discard_iterator& y)
// constexpr discard_iterator& operator-=(iter_difference_t<I> n);
// friend constexpr iter_difference_t<I> operator-(const discard_iterator& x, default_sentinel_t);
// friend constexpr iter_difference_t<I> operator-(default_sentinel_t, const discard_iterator& y);

#include <uscl/iterator>

#include "test_iterators.h"
#include "test_macros.h"

template <class Iter>
_CCCL_CONCEPT MinusEnabled = _CCCL_REQUIRES_EXPR((Iter), Iter& iter)((iter - 1));

__host__ __device__ constexpr bool test()
{
  { // operator-(iter_difference_t<I> n)
    {
      const int index = 3;
      const int diff  = 2;
      cuda::discard_iterator iter(index);
      assert(iter - diff == cuda::discard_iterator(index - diff));
      assert(iter - 0 == cuda::discard_iterator(index));

      static_assert(cuda::std::is_same_v<decltype(iter - 2), cuda::discard_iterator>);
    }

    {
      const int index = 3;
      const int diff  = 2;
      const cuda::discard_iterator iter(index);
      assert(iter - diff == cuda::discard_iterator(index - diff));
      assert(iter - 0 == cuda::discard_iterator(index));

      static_assert(cuda::std::is_same_v<decltype(iter - 2), cuda::discard_iterator>);
    }
  }

  { // operator-(const discard_iterator& x, const discard_iterator& y)
    {
      const int index1 = 4;
      const int index2 = 2;
      cuda::discard_iterator iter1(index1);
      cuda::discard_iterator iter2(index2);
      assert(iter1 - iter2 == index2 - index1);
      assert(iter2 - iter1 == index1 - index2);

      static_assert(cuda::std::is_same_v<decltype(iter1 - iter2), cuda::std::ptrdiff_t>);
    }

    {
      const int index1 = 4;
      const int index2 = 2;
      const cuda::discard_iterator iter1(index1);
      const cuda::discard_iterator iter2(index2);
      assert(iter1 - iter2 == index2 - index1);
      assert(iter2 - iter1 == index1 - index2);

      static_assert(cuda::std::is_same_v<decltype(iter1 - iter2), cuda::std::ptrdiff_t>);
    }
  }

  { // operator-=(iter_difference_t<I> n)
    const int index = 3;
    const int diff  = 2;
    cuda::discard_iterator iter(index);
    assert((iter -= diff) == cuda::discard_iterator(1));
    assert((iter -= 0) == cuda::discard_iterator(1));

    static_assert(cuda::std::is_same_v<decltype(iter -= 2), cuda::discard_iterator&>);
  }

  { // operator-(const discard_iterator& x, default_sentinel_t)
    {
      const int index = 3;
      cuda::discard_iterator iter(index);
      assert((iter - cuda::std::default_sentinel) == -index);

      static_assert(cuda::std::is_same_v<decltype(iter - cuda::std::default_sentinel), cuda::std::ptrdiff_t>);
    }

    {
      const int index = 3;
      const cuda::discard_iterator iter(index);
      assert((iter - cuda::std::default_sentinel) == -index);

      static_assert(cuda::std::is_same_v<decltype(iter - cuda::std::default_sentinel), cuda::std::ptrdiff_t>);
    }
  }

  { // operator-(default_sentinel_t, const discard_iterator& y)
    {
      const int index = 3;
      cuda::discard_iterator iter(index);
      assert((cuda::std::default_sentinel - iter) == index);

      static_assert(cuda::std::is_same_v<decltype(cuda::std::default_sentinel - iter), cuda::std::ptrdiff_t>);
    }

    {
      const int index = 3;
      const cuda::discard_iterator iter(index);
      assert((cuda::std::default_sentinel - iter) == index);

      static_assert(cuda::std::is_same_v<decltype(cuda::std::default_sentinel - iter), cuda::std::ptrdiff_t>);
    }
  }

  return true;
}

int main(int, char**)
{
  test();
  static_assert(test());

  return 0;
}
