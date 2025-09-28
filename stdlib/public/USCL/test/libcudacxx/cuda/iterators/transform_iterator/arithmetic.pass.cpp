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

// transform_iterator::operator{++,--,+=,-=}

#include <uscl/iterator>
#include <uscl/std/cassert>
#include <uscl/std/utility>

#include "test_iterators.h"
#include "test_macros.h"
#include "types.h"

template <class Iter>
_CCCL_CONCEPT can_decrement = _CCCL_REQUIRES_EXPR((Iter), Iter i)((--i));
template <class Iter>
_CCCL_CONCEPT can_post_decrement = _CCCL_REQUIRES_EXPR((Iter), Iter i)((i--));

template <class Iter>
_CCCL_CONCEPT can_plus_equal = _CCCL_REQUIRES_EXPR((Iter), Iter i)((i += 1));
template <class Iter>
_CCCL_CONCEPT can_minus_equal = _CCCL_REQUIRES_EXPR((Iter), Iter i)((i -= 1));

template <class Iter>
__host__ __device__ constexpr void test()
{
  int buffer[8] = {0, 1, 2, 3, 4, 5, 6, 7};

  cuda::transform_iterator iter{Iter{buffer}, PlusOne{}};
  assert((++iter).base() == Iter{buffer + 1});

  if constexpr (cuda::std::forward_iterator<Iter>)
  {
    assert((iter++).base() == Iter{buffer + 1});
  }
  else
  {
    iter++;
    static_assert(cuda::std::is_same_v<decltype(iter++), void>);
  }
  assert(iter.base() == Iter{buffer + 2});

  if constexpr (cuda::std::bidirectional_iterator<Iter>)
  {
    assert((--iter).base() == Iter{buffer + 1});
    assert((iter--).base() == Iter{buffer + 1});
    assert(iter.base() == Iter{buffer});
  }
  else
  {
    static_assert(!can_decrement<Iter>);
    static_assert(!can_post_decrement<Iter>);
  }

  if constexpr (cuda::std::random_access_iterator<Iter>)
  {
    assert((iter += 4).base() == Iter{buffer + 4});
    assert((iter -= 3).base() == Iter{buffer + 1});
  }
  else
  {
    static_assert(!can_plus_equal<Iter>);
    static_assert(!can_minus_equal<Iter>);
  }
}

__host__ __device__ constexpr bool test()
{
  test<cpp17_input_iterator<int*>>();
  test<forward_iterator<int*>>();
  test<bidirectional_iterator<int*>>();
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
