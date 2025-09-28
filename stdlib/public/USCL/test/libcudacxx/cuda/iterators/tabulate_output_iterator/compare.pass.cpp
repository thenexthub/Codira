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

// operator{<,>,<=,>=,==,!=,<=>}

#include <uscl/iterator>
#if _LIBCUDACXX_HAS_SPACESHIP_OPERATOR()
#  include <cuda/std/compare>
#endif // _LIBCUDACXX_HAS_SPACESHIP_OPERATOR()

#include "test_macros.h"
#include "types.h"

__host__ __device__ constexpr bool test()
{
  cuda::tabulate_output_iterator iter1{basic_functor{}, 42};
  const auto iter2 = iter1 + 1;

  assert(!(iter1 < iter1));
  assert(iter1 < iter2);
  assert(!(iter2 < iter1));
  assert(iter1 <= iter1);
  assert(iter1 <= iter2);
  assert(!(iter2 <= iter1));
  assert(!(iter1 > iter1));
  assert(!(iter1 > iter2));
  assert(iter2 > iter1);
  assert(iter1 >= iter1);
  assert(!(iter1 >= iter2));
  assert(iter2 >= iter1);
  assert(iter1 == iter1);
  assert(!(iter1 == iter2));
  assert(iter2 == iter2);
  assert(!(iter1 != iter1));
  assert(iter1 != iter2);
  assert(!(iter2 != iter2));

#if TEST_HAS_SPACESHIP()
  static_assert(cuda::std::three_way_comparable<cuda::tabulate_output_iterator<int>>);
  assert((iter1 <=> iter2) == cuda::std::strong_ordering::less);
  assert((iter1 <=> iter1) == cuda::std::strong_ordering::equal);
  assert((iter2 <=> iter1) == cuda::std::strong_ordering::greater);
#endif // TEST_HAS_SPACESHIP()

  return true;
}

int main(int, char**)
{
  test();
  static_assert(test(), "");

  return 0;
}
