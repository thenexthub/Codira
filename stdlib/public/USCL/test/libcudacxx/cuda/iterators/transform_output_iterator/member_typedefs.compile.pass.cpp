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

// Test iterator category and iterator concepts.

#include <uscl/iterator>
#include <uscl/std/cassert>
#include <uscl/std/cstdint>

#include "test_iterators.h"
#include "test_macros.h"
#include "types.h"

__host__ __device__ void test()
{
  {
    using Iter = cuda::transform_output_iterator<int*, PlusOne>;
    static_assert(cuda::std::same_as<Iter::iterator_concept, cuda::std::output_iterator_tag>);
    static_assert(cuda::std::same_as<Iter::iterator_category, cuda::std::output_iterator_tag>);
    static_assert(cuda::std::same_as<Iter::pointer, void>);
    static_assert(cuda::std::same_as<Iter::reference, void>);
    static_assert(cuda::std::same_as<Iter::value_type, void>);
    static_assert(cuda::std::same_as<Iter::difference_type, cuda::std::ptrdiff_t>);
    static_assert(cuda::std::output_iterator<Iter, int>);
  }

  {
    using Iter = cuda::transform_output_iterator<random_access_iterator<int*>, PlusOne>;
    static_assert(cuda::std::same_as<Iter::iterator_concept, cuda::std::output_iterator_tag>);
    static_assert(cuda::std::same_as<Iter::iterator_category, cuda::std::output_iterator_tag>);
    static_assert(cuda::std::same_as<Iter::pointer, void>);
    static_assert(cuda::std::same_as<Iter::reference, void>);
    static_assert(cuda::std::same_as<Iter::value_type, void>);
    static_assert(cuda::std::same_as<Iter::difference_type, cuda::std::ptrdiff_t>);
    static_assert(cuda::std::output_iterator<Iter, int>);
  }
}

int main(int, char**)
{
  return 0;
}
