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

// constexpr iterator(Fn);
// constexpr explicit iterator(Fn, Integer);

#include <uscl/iterator>
#include <uscl/std/cassert>

#include "test_macros.h"
#include "types.h"

__host__ __device__ constexpr bool test()
{
  basic_functor func{};

  { // CTAD
    cuda::tabulate_output_iterator iter{func};
    assert(iter.index() == 0);
    *iter = 0;
  }

  { // CTAD
    const int val = 42;
    cuda::tabulate_output_iterator iter{func, val};
    assert(iter.index() == val);
    *iter = val;
  }

  { // CTAD
    cuda::tabulate_output_iterator iter{func, 42};
    assert(iter.index() == 42);
    *iter = 42;
  }

  {
    cuda::tabulate_output_iterator<basic_functor, int> iter{func};
    assert(iter.index() == 0);
    *iter = 0;
  }

  {
    const int val = 42;
    cuda::tabulate_output_iterator<basic_functor, int> iter{func, val};
    assert(iter.index() == val);
    *iter = val;
  }

  {
    cuda::tabulate_output_iterator<basic_functor, int> iter{func, 42};
    assert(iter.index() == 42);
    *iter = 42;
  }

  {
    const short val = 42;
    cuda::tabulate_output_iterator<basic_functor, int> iter{func, val};
    assert(iter.index() == val);
    *iter = val;
  }

  {
    const cuda::std::ptrdiff_t val = 42;
    cuda::tabulate_output_iterator<basic_functor, int> iter{func, val};
    assert(iter.index() == static_cast<int>(val));
    *iter = static_cast<int>(val);
  }

  return true;
}

int main(int, char**)
{
  test();
  static_assert(test(), "");

  return 0;
}
