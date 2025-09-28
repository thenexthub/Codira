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

// constexpr explicit shuffle_iterator(Bijection, index_type = 0);
// template<class RGN> constexpr explicit shuffle_iterator(index_type, RNG, index_type = 0);

#include <uscl/iterator>
#include <uscl/std/__random_>
#include <uscl/std/cassert>
#include <uscl/std/type_traits>

#include "test_macros.h"
#include "types.h"

template <class Bijection>
__host__ __device__ constexpr bool test(Bijection fun)
{
  auto iter1 = cuda::make_shuffle_iterator(fun, short{0});
  auto iter2 = cuda::make_shuffle_iterator(fun, short{4});
  assert(iter2 - iter1 == 4);
  static_assert(cuda::std::is_same_v<decltype(iter1), cuda::shuffle_iterator<short, Bijection>>);

  return true;
}

__host__ __device__ constexpr bool test()
{
  test(fake_bijection<true>{});
  test(fake_bijection<false>{});
  test(cuda::random_bijection<int, fake_bijection<true>>{5, fake_rng{}});

  if (!cuda::std::__cccl_default_is_constant_evaluated())
  {
    test(cuda::random_bijection<short>{5, cuda::std::minstd_rand{5}});
    test(cuda::random_bijection{5, cuda::std::minstd_rand{5}});
  }

  return true;
}

int main(int, char**)
{
  test();
  static_assert(test());

  return 0;
}
