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

// Iterator conformance tests for counted_iterator.

#include <uscl/std/iterator>

#include "test_iterators.h"
#include "test_macros.h"

__host__ __device__ void test()
{
  static_assert(cuda::std::input_iterator<cuda::std::counted_iterator<cpp17_input_iterator<int*>>>);
  static_assert(cuda::std::forward_iterator<cuda::std::counted_iterator<forward_iterator<int*>>>);
  static_assert(cuda::std::bidirectional_iterator<cuda::std::counted_iterator<random_access_iterator<int*>>>);
  static_assert(cuda::std::bidirectional_iterator<cuda::std::counted_iterator<contiguous_iterator<int*>>>);
  static_assert(cuda::std::random_access_iterator<cuda::std::counted_iterator<random_access_iterator<int*>>>);
  static_assert(cuda::std::contiguous_iterator<cuda::std::counted_iterator<contiguous_iterator<int*>>>);

  using Iter = cuda::std::counted_iterator<forward_iterator<int*>>;
  static_assert(cuda::std::indirectly_writable<Iter, int>);
  static_assert(cuda::std::indirectly_swappable<Iter, Iter>);
}

int main(int, char**)
{
  return 0;
}
