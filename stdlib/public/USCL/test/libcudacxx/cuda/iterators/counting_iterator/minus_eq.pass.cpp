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

// constexpr iterator& operator-=(difference_type n)
//   requires advanceable<W>;

#include <uscl/iterator>
#include <uscl/std/cassert>

#include "test_macros.h"
#include "types.h"

__host__ __device__ constexpr bool test()
{
  { // When "_Start" is signed integer like.
    cuda::counting_iterator<int> iter1{10};
    cuda::counting_iterator<int> iter2{10};
    assert(iter1 == iter2);
    iter1 -= 0;
    assert(iter1 == iter2);
    iter1 -= 5;
    assert(iter1 != iter2);
    assert(iter1 == cuda::std::ranges::prev(iter2, 5));

    static_assert(noexcept(iter2 -= 5));
    static_assert(cuda::std::is_reference_v<decltype(iter2 -= 5)>);
  }

  { // When "_Start" is not integer like.
    cuda::counting_iterator<SomeInt> iter1{SomeInt{10}};
    cuda::counting_iterator<SomeInt> iter2{SomeInt{10}};
    assert(iter1 == iter2);
    iter1 -= 0;
    assert(iter1 == iter2);
    iter1 -= 5;
    assert(iter1 != iter2);
    assert(iter1 == cuda::std::ranges::prev(iter2, 5));

    static_assert(!noexcept(iter2 -= 5));
    static_assert(cuda::std::is_reference_v<decltype(iter2 -= 5)>);
  }

  { // When "_Start" is unsigned integer like and n is greater than or equal to zero.
    cuda::counting_iterator<unsigned> iter1{10};
    cuda::counting_iterator<unsigned> iter2{10};
    assert(iter1 == iter2);
    iter1 -= 0;
    assert(iter1 == iter2);
    iter1 -= 5;
    assert(iter1 != iter2);
    assert(iter1 == cuda::std::ranges::prev(iter2, 5));

    static_assert(noexcept(iter2 -= 5));
    static_assert(cuda::std::is_reference_v<decltype(iter2 -= 5)>);
  }

  { // When "_Start" is unsigned integer like and n is less than zero.
    using difference_type = typename cuda::counting_iterator<unsigned>::difference_type;
    cuda::counting_iterator<unsigned> iter1{10};
    cuda::counting_iterator<unsigned> iter2{10};
    assert(iter1 == iter2);
    iter1 -= difference_type(-5);
    assert(iter1 != iter2);
    assert(iter1 == cuda::std::ranges::next(iter2, 5));

    static_assert(noexcept(iter2 -= -5));
    static_assert(cuda::std::is_reference_v<decltype(iter2 -= difference_type(-5))>);
  }

  return true;
}

int main(int, char**)
{
  test();
  static_assert(test(), "");

  return 0;
}
