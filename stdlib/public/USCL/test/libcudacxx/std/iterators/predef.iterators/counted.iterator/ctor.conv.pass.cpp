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

// template<class I2>
//   requires convertible_to<const I2&, I>
//     constexpr counted_iterator(const counted_iterator<I2>& x);

#include <uscl/std/iterator>

#include "test_iterators.h"
#include "test_macros.h"

template <class T>
class ConvertibleTo
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

  ConvertibleTo() = default;
  __host__ __device__ explicit constexpr ConvertibleTo(int* it)
      : it_(it)
  {}

  __host__ __device__ constexpr reference operator*() const
  {
    return *it_;
  }

  __host__ __device__ constexpr ConvertibleTo& operator++()
  {
    ++it_;
    return *this;
  }
  __host__ __device__ constexpr ConvertibleTo operator++(int)
  {
    ConvertibleTo tmp(*this);
    ++(*this);
    return tmp;
  }

  __host__ __device__ constexpr operator T() const
  {
    return T(it_);
  }
};

__host__ __device__ constexpr bool test()
{
  int buffer[8] = {1, 2, 3, 4, 5, 6, 7, 8};

  {
    static_assert(cuda::std::is_constructible_v<cuda::std::counted_iterator<forward_iterator<int*>>,
                                                cuda::std::counted_iterator<forward_iterator<int*>>>);
    static_assert(!cuda::std::is_constructible_v<cuda::std::counted_iterator<forward_iterator<int*>>,
                                                 cuda::std::counted_iterator<random_access_iterator<int*>>>);
  }
  {
    cuda::std::counted_iterator iter1(ConvertibleTo<forward_iterator<int*>>{buffer}, 8);
    cuda::std::counted_iterator<forward_iterator<int*>> iter2(iter1);
    assert(iter2.base() == forward_iterator<int*>{buffer});
    assert(iter2.count() == 8);
  }
  {
    const cuda::std::counted_iterator iter1(ConvertibleTo<forward_iterator<int*>>{buffer}, 8);
    const cuda::std::counted_iterator<forward_iterator<int*>> iter2(iter1);
    assert(iter2.base() == forward_iterator<int*>{buffer});
    assert(iter2.count() == 8);
  }

  return true;
}

int main(int, char**)
{
  test();
  static_assert(test());

  return 0;
}
