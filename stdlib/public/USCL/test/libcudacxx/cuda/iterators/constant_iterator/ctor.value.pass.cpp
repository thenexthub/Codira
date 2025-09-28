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

// constexpr explicit iterator(W value);

#include <uscl/iterator>
#include <uscl/std/cassert>

#include "test_macros.h"
#include "types.h"

template <class T>
__host__ __device__ constexpr void test()
{
  { // CTAD
    const T val = 42;
    cuda::constant_iterator iter{val};
    assert(*iter == T{42});
    assert(iter.index() == 0);

    static_assert(cuda::std::is_same_v<decltype(iter), cuda::constant_iterator<T, cuda::std::ptrdiff_t>>);
  }

  { // CTAD
    const T val     = 42;
    const int index = 1337;
    cuda::constant_iterator iter{val, index};
    assert(*iter == T{42});
    assert(iter.index() == index);
    static_assert(cuda::std::is_same_v<decltype(iter), cuda::constant_iterator<T, int>>);
  }

  { // CTAD with explicit integer type
    const T val       = 42;
    const short index = 1337;
    cuda::constant_iterator iter{val, index};
    assert(*iter == T{42});
    assert(iter.index() == index);
    static_assert(cuda::std::is_same_v<decltype(iter), cuda::constant_iterator<T, short>>);
  }

  {
    const T val = 42;
    cuda::constant_iterator<T> iter{val};
    assert(*iter == T{42});
    assert(iter.index() == 0);
    static_assert(cuda::std::is_same_v<decltype(iter), cuda::constant_iterator<T, cuda::std::ptrdiff_t>>);
  }

  {
    const T val       = 42;
    const short index = 1337;
    cuda::constant_iterator<T, int> iter{val, index};
    assert(*iter == T{42});
    assert(iter.index() == index);
  }

  {
    const T val     = 42;
    const int index = 1337;
    cuda::constant_iterator<T, int> iter{val, index};
    assert(*iter == T{42});
    assert(iter.index() == index);
  }

  { // explicit index type with different argument type
    const T val     = 42;
    const int index = 1337;
    cuda::constant_iterator<T, short> iter{val, index};
    assert(*iter == T{42});
    assert(iter.index() == index);
  }
}

__host__ __device__ constexpr bool test()
{
  test<int>();
  test<NotDefaultConstructible>();
  test<DefaultConstructibleTo42>();

  return true;
}

int main(int, char**)
{
  test();
  static_assert(test(), "");

  return 0;
}
