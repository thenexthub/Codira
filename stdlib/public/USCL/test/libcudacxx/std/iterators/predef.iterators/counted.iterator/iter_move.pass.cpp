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

// friend constexpr iter_rvalue_reference_t<I>
//   iter_move(const counted_iterator& i)
//     noexcept(noexcept(ranges::iter_move(i.current)))
//     requires input_iterator<I>;

#include <uscl/std/iterator>

#include "test_iterators.h"
#include "test_macros.h"

template <bool IsNoexcept>
class HasNoexceptIterMove
{
  int* it_;

public:
  typedef cuda::std::input_iterator_tag iterator_category;
  typedef int value_type;
  typedef typename cuda::std::iterator_traits<int*>::difference_type difference_type;
  typedef int* pointer;
  typedef int& reference;

  __host__ __device__ constexpr int* base() const
  {
    return it_;
  }

  HasNoexceptIterMove() = default;
  __host__ __device__ explicit constexpr HasNoexceptIterMove(int* it)
      : it_(it)
  {}

  __host__ __device__ constexpr reference operator*() const noexcept(IsNoexcept)
  {
    return *it_;
  }

  __host__ __device__ constexpr HasNoexceptIterMove& operator++()
  {
    ++it_;
    return *this;
  }
  __host__ __device__ constexpr HasNoexceptIterMove operator++(int)
  {
    HasNoexceptIterMove tmp(*this);
    ++(*this);
    return tmp;
  }
};

__host__ __device__ constexpr bool test()
{
  int buffer[8] = {1, 2, 3, 4, 5, 6, 7, 8};

  {
    auto iter1       = cpp17_input_iterator<int*>(buffer);
    auto commonIter1 = cuda::std::counted_iterator<decltype(iter1)>(iter1, 8);
    assert(cuda::std::ranges::iter_move(commonIter1) == 1);
    static_assert(cuda::std::is_same_v<decltype(cuda::std::ranges::iter_move(commonIter1)), int&&>);
  }
  {
    auto iter1       = forward_iterator<int*>(buffer);
    auto commonIter1 = cuda::std::counted_iterator<decltype(iter1)>(iter1, 8);
    assert(cuda::std::ranges::iter_move(commonIter1) == 1);
    static_assert(cuda::std::is_same_v<decltype(cuda::std::ranges::iter_move(commonIter1)), int&&>);
  }
  {
    auto iter1       = random_access_iterator<int*>(buffer);
    auto commonIter1 = cuda::std::counted_iterator<decltype(iter1)>(iter1, 8);
    assert(cuda::std::ranges::iter_move(commonIter1) == 1);
    static_assert(cuda::std::is_same_v<decltype(cuda::std::ranges::iter_move(commonIter1)), int&&>);
  }

  // Test noexceptness.
  {
    static_assert(noexcept(
      cuda::std::ranges::iter_move(cuda::std::declval<cuda::std::counted_iterator<HasNoexceptIterMove<true>>>())));
    static_assert(!noexcept(
      cuda::std::ranges::iter_move(cuda::std::declval<cuda::std::counted_iterator<HasNoexceptIterMove<false>>>())));
  }

  return true;
}

int main(int, char**)
{
  test();
  static_assert(test());

  return 0;
}
