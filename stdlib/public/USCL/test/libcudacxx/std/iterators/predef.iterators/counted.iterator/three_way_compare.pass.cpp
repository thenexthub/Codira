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
// UNSUPPORTED: c++17
// XFAIL: c++20

// template<common_with<I> I2>
//   friend constexpr strong_ordering operator<=>(
//     const counted_iterator& x, const counted_iterator<I2>& y);

#include <uscl/std/iterator>

#include "test_iterators.h"
#include "test_macros.h"

// This iterator is common_with forward_iterator but NOT comparable with it.
template <class It>
class CommonWithForwardIter
{
  It it_;

public:
  typedef cuda::std::input_iterator_tag iterator_category;
  typedef typename cuda::std::iterator_traits<It>::value_type value_type;
  typedef typename cuda::std::iterator_traits<It>::difference_type difference_type;
  typedef It pointer;
  typedef typename cuda::std::iterator_traits<It>::reference reference;

  __host__ __device__ constexpr It base() const
  {
    return it_;
  }

  CommonWithForwardIter() = default;
  __host__ __device__ explicit constexpr CommonWithForwardIter(It it)
      : it_(it)
  {}
  __host__ __device__ constexpr CommonWithForwardIter(const forward_iterator<It>& it)
      : it_(it.base())
  {}

  __host__ __device__ constexpr reference operator*() const
  {
    return *it_;
  }

  __host__ __device__ constexpr CommonWithForwardIter& operator++()
  {
    ++it_;
    return *this;
  }
  __host__ __device__ constexpr CommonWithForwardIter operator++(int)
  {
    CommonWithForwardIter tmp(*this);
    ++(*this);
    return tmp;
  }
};

__host__ __device__ constexpr bool test()
{
  int buffer[8] = {1, 2, 3, 4, 5, 6, 7, 8};

  auto& Eq      = cuda::std::strong_ordering::equal;
  auto& Less    = cuda::std::strong_ordering::less;
  auto& Greater = cuda::std::strong_ordering::greater;

  {
    {
      cuda::std::counted_iterator iter1(forward_iterator<int*>(buffer), 8);
      cuda::std::counted_iterator iter2(CommonWithForwardIter<int*>(buffer), 8);

      assert((iter1 <=> iter2) == Eq);
      assert((iter2 <=> iter1) == Eq);
      ++iter1;
      assert((iter1 <=> iter2) == Greater);
      assert((iter2 <=> iter1) == Less);
    }
    {
      cuda::std::counted_iterator iter1(forward_iterator<int*>(buffer), 8);
      cuda::std::counted_iterator iter2(forward_iterator<int*>(buffer), 8);

      assert((iter1 <=> iter2) == Eq);
      assert((iter2 <=> iter1) == Eq);
      ++iter1;
      assert((iter1 <=> iter2) == Greater);
      assert((iter2 <=> iter1) == Less);
    }
  }

  return true;
}

int main(int, char**)
{
  test();
  static_assert(test());

  return 0;
}
