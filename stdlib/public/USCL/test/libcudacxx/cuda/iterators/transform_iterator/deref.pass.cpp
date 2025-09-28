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

// transform_iterator::operator*

#include <uscl/iterator>
#include <uscl/std/cassert>
#include <uscl/std/type_traits>
#include <uscl/std/utility>

#include "test_iterators.h"
#include "test_macros.h"
#include "types.h"

template <class Iter>
__host__ __device__ constexpr void test()
{
  int buffer[8] = {0, 1, 2, 3, 4, 5, 6, 7};

  {
    cuda::transform_iterator iter{Iter{buffer}, PlusOne{}};
    assert(*iter == 1);
    assert(*cuda::std::as_const(iter) == 1);
    assert(*buffer == 0);

    static_assert(!noexcept(*iter));
    static_assert(cuda::std::is_same_v<int, decltype(*iter)>);
  }

  {
    cuda::transform_iterator iter{Iter{buffer}, PlusOneMutable{}};
    assert(*iter == 1);
    assert(*buffer == 0);

    static_assert(!noexcept(*iter));
    static_assert(cuda::std::is_same_v<int, decltype(*iter)>);
  }

  {
    cuda::transform_iterator iter{Iter{buffer}, PlusOneNoexcept{}};
    assert(*iter == 1);
    assert(*buffer == 0);

    static_assert(noexcept(*iter) == noexcept(*cuda::std::declval<Iter>()));
    static_assert(cuda::std::is_same_v<int, decltype(*iter)>);
  }

  {
    cuda::transform_iterator iter{Iter{buffer}, Increment{}};
    assert(*iter == 1);
    assert(*buffer == 1);

    static_assert(!noexcept(*iter));
    static_assert(cuda::std::is_same_v<int&, decltype(*iter)>);
  }

  {
    cuda::transform_iterator iter{Iter{buffer}, IncrementRvalueRef{}};
    assert(*iter == 2);
    assert(*buffer == 2);

    static_assert(!noexcept(*iter));
    static_assert(cuda::std::is_same_v<int&&, decltype(*iter)>);
  }

  {
    cuda::transform_iterator iter{Iter{buffer}, PlusWithMutableMember{3}};
    assert(*iter == 5);
    assert(*iter == 6);
    assert(*iter == 7);
    assert(*buffer == 2);

    static_assert(noexcept(*iter) == noexcept(*cuda::std::declval<Iter>()));
    static_assert(cuda::std::is_same_v<int, decltype(*iter)>);
  }
}

__host__ __device__ constexpr bool test()
{
  test<cpp17_input_iterator<int*>>();
  test<forward_iterator<int*>>();
  test<random_access_iterator<int*>>();
  test<int*>();

  return true;
}

int main(int, char**)
{
  test();
  static_assert(test(), "");

  return 0;
}
