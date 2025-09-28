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

// template<indirectly_swappable<I> I2>
//   friend constexpr void
//     iter_swap(const counted_iterator& x, const counted_iterator<I2>& y)
//       noexcept(noexcept(ranges::iter_swap(x.current, y.current)));

#include <uscl/std/iterator>

#include "test_iterators.h"
#include "test_macros.h"

template <bool IsNoexcept>
class HasNoexceptIterSwap
{
  int* it_;

public:
  typedef cuda::std::input_iterator_tag iterator_category;
  typedef int value_type;
  typedef int element_type;
  typedef typename cuda::std::iterator_traits<int*>::difference_type difference_type;
  typedef int* pointer;
  typedef int& reference;

  __host__ __device__ constexpr int* base() const
  {
    return it_;
  }

  HasNoexceptIterSwap() = default;
  __host__ __device__ __host__ __device__ explicit constexpr HasNoexceptIterSwap(int* it)
      : it_(it)
  {}

  __host__ __device__ constexpr reference operator*() const
  {
    return *it_;
  }

  __host__ __device__ constexpr HasNoexceptIterSwap& operator++()
  {
    ++it_;
    return *this;
  }
  __host__ __device__ constexpr HasNoexceptIterSwap operator++(int)
  {
    HasNoexceptIterSwap tmp(*this);
    ++(*this);
    return tmp;
  }

  __host__ __device__ friend void iter_swap(const HasNoexceptIterSwap&, const HasNoexceptIterSwap&) noexcept(IsNoexcept)
  {}
};

__host__ __device__ constexpr bool test()
{
  int buffer[8] = {1, 2, 3, 4, 5, 6, 7, 8};

  {
    auto iter1       = cpp17_input_iterator<int*>(buffer);
    auto commonIter1 = cuda::std::counted_iterator<decltype(iter1)>(iter1, 8);
    auto commonIter2 = cuda::std::counted_iterator<decltype(iter1)>(iter1, 8);
    for (auto i = 0; i < 4; ++i)
    {
      ++commonIter2;
    }
    assert(*commonIter2 == 5);
    cuda::std::ranges::iter_swap(commonIter1, commonIter2);
    assert(*commonIter1 == 5);
    assert(*commonIter2 == 1);
    cuda::std::ranges::iter_swap(commonIter2, commonIter1);
  }
  {
    auto iter1       = forward_iterator<int*>(buffer);
    auto commonIter1 = cuda::std::counted_iterator<decltype(iter1)>(iter1, 8);
    auto commonIter2 = cuda::std::counted_iterator<decltype(iter1)>(iter1, 8);
    for (auto i = 0; i < 4; ++i)
    {
      ++commonIter2;
    }
    assert(*commonIter2 == 5);
    cuda::std::ranges::iter_swap(commonIter1, commonIter2);
    assert(*commonIter1 == 5);
    assert(*commonIter2 == 1);
    cuda::std::ranges::iter_swap(commonIter2, commonIter1);
  }
  {
    auto iter1       = random_access_iterator<int*>(buffer);
    auto commonIter1 = cuda::std::counted_iterator<decltype(iter1)>(iter1, 8);
    auto commonIter2 = cuda::std::counted_iterator<decltype(iter1)>(iter1, 8);
    for (auto i = 0; i < 4; ++i)
    {
      ++commonIter2;
    }
    assert(*commonIter2 == 5);
    cuda::std::ranges::iter_swap(commonIter1, commonIter2);
    assert(*commonIter1 == 5);
    assert(*commonIter2 == 1);
    cuda::std::ranges::iter_swap(commonIter2, commonIter1);
  }

  // Test noexceptness.
  {
    static_assert(noexcept(cuda::std::ranges::iter_swap(
      cuda::std::declval<cuda::std::counted_iterator<HasNoexceptIterSwap<true>>&>(),
      cuda::std::declval<cuda::std::counted_iterator<HasNoexceptIterSwap<true>>&>())));
    static_assert(!noexcept(cuda::std::ranges::iter_swap(
      cuda::std::declval<cuda::std::counted_iterator<HasNoexceptIterSwap<false>>&>(),
      cuda::std::declval<cuda::std::counted_iterator<HasNoexceptIterSwap<false>>&>())));
  }

  return true;
}

int main(int, char**)
{
  test();
  static_assert(test());

  return 0;
}
