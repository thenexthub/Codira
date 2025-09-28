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

// iterator() requires default_initializable<Fn> = default;

#include <uscl/iterator>
#include <uscl/std/cassert>

#include "test_iterators.h"
#include "test_macros.h"
#include "types.h"

__host__ __device__ constexpr bool test()
{
  {
    cuda::transform_input_output_iterator<random_access_iterator<int*>, PlusOne, PlusOne> iter;
    assert(iter.base() == random_access_iterator<int*>{});
  }

  {
    const cuda::transform_input_output_iterator<random_access_iterator<int*>, PlusOne, PlusOne> iter;
    assert(iter.base() == random_access_iterator<int*>{});
  }

  {
    static_assert(
      !cuda::std::is_default_constructible_v<
        cuda::transform_input_output_iterator<random_access_iterator<int*>, NotDefaultConstructiblePlusOne, PlusOne>>);
    static_assert(
      !cuda::std::is_default_constructible_v<
        cuda::transform_input_output_iterator<random_access_iterator<int*>, PlusOne, NotDefaultConstructiblePlusOne>>);
  }

  return true;
}

int main(int, char**)
{
  test();
  static_assert(test(), "");

  return 0;
}
