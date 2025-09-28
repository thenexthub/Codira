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

// iterator() requires default_initializable<W> = default;

#include <uscl/iterator>
#include <uscl/std/cassert>

#include "test_macros.h"
#include "types.h"

__host__ __device__ constexpr bool test()
{
  {
    [[maybe_unused]] cuda::shuffle_iterator iter;
    static_assert(
      cuda::std::is_same_v<decltype(iter),
                           cuda::shuffle_iterator<size_t, cuda::random_bijection<size_t, cuda::__feistel_bijection>>>);
  }

  {
    [[maybe_unused]] cuda::shuffle_iterator iter{};
    static_assert(
      cuda::std::is_same_v<decltype(iter),
                           cuda::shuffle_iterator<size_t, cuda::random_bijection<size_t, cuda::__feistel_bijection>>>);
  }

  {
    [[maybe_unused]] cuda::shuffle_iterator<int> iter;
    static_assert(
      cuda::std::is_same_v<decltype(iter),
                           cuda::shuffle_iterator<int, cuda::random_bijection<int, cuda::__feistel_bijection>>>);
  }

  {
    [[maybe_unused]] cuda::shuffle_iterator<int> iter{};
    static_assert(
      cuda::std::is_same_v<decltype(iter),
                           cuda::shuffle_iterator<int, cuda::random_bijection<int, cuda::__feistel_bijection>>>);
  }

  {
    [[maybe_unused]] cuda::shuffle_iterator<int, cuda::random_bijection<size_t>> iter;
    static_assert(
      cuda::std::is_same_v<decltype(iter),
                           cuda::shuffle_iterator<int, cuda::random_bijection<size_t, cuda::__feistel_bijection>>>);
  }

  {
    [[maybe_unused]] cuda::shuffle_iterator<int, cuda::random_bijection<size_t>> iter{};
    static_assert(
      cuda::std::is_same_v<decltype(iter),
                           cuda::shuffle_iterator<int, cuda::random_bijection<size_t, cuda::__feistel_bijection>>>);
  }

  return true;
}

int main(int, char**)
{
  test();
  static_assert(test());

  return 0;
}
