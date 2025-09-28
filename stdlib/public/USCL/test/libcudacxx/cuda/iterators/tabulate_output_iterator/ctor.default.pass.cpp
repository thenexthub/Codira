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

#include "test_macros.h"
#include "types.h"

__host__ __device__ constexpr bool test()
{
  {
    cuda::tabulate_output_iterator<basic_functor> iter;
    assert(iter.index() == 0);
    *iter = 0;
    static_assert(
      cuda::std::is_same_v<decltype(iter), cuda::tabulate_output_iterator<basic_functor, cuda::std::ptrdiff_t>>);
  }

  {
    const cuda::tabulate_output_iterator<basic_functor> iter;
    assert(iter.index() == 0);
    *iter = 0;
    static_assert(
      cuda::std::is_same_v<decltype(iter), const cuda::tabulate_output_iterator<basic_functor, cuda::std::ptrdiff_t>>);
  }

  {
    cuda::tabulate_output_iterator<mutable_functor> iter;
    assert(iter.index() == 0);
    *iter = 0;
    static_assert(
      cuda::std::is_same_v<decltype(iter), cuda::tabulate_output_iterator<mutable_functor, cuda::std::ptrdiff_t>>);
  }

  {
    static_assert(
      !cuda::std::is_default_constructible_v<cuda::tabulate_output_iterator<not_default_constructible_functor>>);
  }

  return true;
}

int main(int, char**)
{
  test();
  static_assert(test(), "");

  return 0;
}
