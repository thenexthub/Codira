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

// friend constexpr bool operator==(const discard_iterator& x, const discard_iterator& y);
// friend constexpr bool operator==(const discard_iterator& x, default_sentinel_t);

#include <uscl/iterator>
#include <uscl/std/utility>

#include "test_macros.h"

__host__ __device__ constexpr bool test()
{
  const int offset1 = 3;
  const int offset2 = 4;
  cuda::discard_iterator iter1(offset1);
  cuda::discard_iterator iter2(offset2);

  // equality
  assert(iter1 == iter1);
  assert(iter1 != iter2);

  assert(cuda::std::as_const(iter1) == iter1);
  assert(iter1 != cuda::std::as_const(iter2));

  assert(iter1 == cuda::std::as_const(iter1));
  assert(cuda::std::as_const(iter1) != iter2);

  assert(cuda::std::as_const(iter1) == cuda::std::as_const(iter1));
  assert(cuda::std::as_const(iter1) != cuda::std::as_const(iter2));

  // relation
  assert(iter1 < iter2);
  assert(iter1 <= iter2);
  assert(iter2 > iter1);
  assert(iter2 >= iter1);

  assert(cuda::std::as_const(iter1) < iter2);
  assert(cuda::std::as_const(iter1) <= iter2);
  assert(cuda::std::as_const(iter2) > iter1);
  assert(cuda::std::as_const(iter2) >= iter1);

  assert(iter1 < cuda::std::as_const(iter2));
  assert(iter1 <= cuda::std::as_const(iter2));
  assert(iter2 > cuda::std::as_const(iter1));
  assert(iter2 >= cuda::std::as_const(iter1));

  assert(cuda::std::as_const(iter1) < cuda::std::as_const(iter2));
  assert(cuda::std::as_const(iter1) <= cuda::std::as_const(iter2));
  assert(cuda::std::as_const(iter2) > cuda::std::as_const(iter1));
  assert(cuda::std::as_const(iter2) >= cuda::std::as_const(iter1));

#if _LIBCUDACXX_HAS_SPACESHIP_OPERATOR()
  using cuda::std::strong_ordering::equal;
  using cuda::std::strong_ordering::greater;
  using cuda::std::strong_ordering::less;

  assert(iter1 <=> iter1 == equal);
  assert(iter2 <=> iter1 == greater);
  assert(iter1 <=> iter2 == less);

  assert(cuda::std::as_const(iter1) <=> iter1 == equal);
  assert(cuda::std::as_const(iter2) <=> iter1 == greater);
  assert(cuda::std::as_const(iter1) <=> iter2 == less);

  assert(iter1 <=> cuda::std::as_const(iter1) == equal);
  assert(iter2 <=> cuda::std::as_const(iter1) == greater);
  assert(iter1 <=> cuda::std::as_const(iter2) == less);

  assert(cuda::std::as_const(iter1) <=> cuda::std::as_const(iter1) == equal);
  assert(cuda::std::as_const(iter2) <=> cuda::std::as_const(iter1) == greater);
  assert(cuda::std::as_const(iter1) <=> cuda::std::as_const(iter2) == less);
#endif // _LIBCUDACXX_HAS_SPACESHIP_OPERATOR()

  return true;
}

int main(int, char**)
{
  test();
  static_assert(test());

  return 0;
}
