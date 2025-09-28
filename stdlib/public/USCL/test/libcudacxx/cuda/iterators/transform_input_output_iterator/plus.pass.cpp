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

// friend constexpr iterator operator+(iterator i, difference_type n);
// friend constexpr iterator operator+(difference_type n, iterator i);

#include <uscl/iterator>
#include <uscl/std/cassert>

#include "test_macros.h"
#include "types.h"

__host__ __device__ constexpr bool test()
{
  int buffer[8] = {0, 1, 2, 3, 4, 5, 6, 7};
  PlusOne input_func{};
  TimesTwo output_func{};

  { // <iterator> + difference_type
    cuda::transform_input_output_iterator iter1{buffer + 1, input_func, output_func};
    cuda::transform_input_output_iterator iter2{buffer + 1, input_func, output_func};
    assert(iter1 == iter2);
    assert(iter1 + 0 == iter2);
    assert(iter1 + 5 != iter2);
    assert((iter1 + 5).base() == buffer + 6);

    static_assert(noexcept(iter2 + 5));
    static_assert(!cuda::std::is_reference_v<decltype(iter2 + 5)>);
  }

  { // difference_type + <iterator>
    cuda::transform_input_output_iterator iter1{buffer + 1, input_func, output_func};
    cuda::transform_input_output_iterator iter2{buffer + 1, input_func, output_func};
    assert(0 + iter1 == iter2);
    assert(5 + iter1 != iter2);
    assert((5 + iter1).base() == buffer + 6);

    static_assert(noexcept(5 + iter2));
    static_assert(!cuda::std::is_reference_v<decltype(5 + iter2)>);
  }

  return true;
}

int main(int, char**)
{
  test();
  static_assert(test(), "");

  return 0;
}
