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
//   requires assignable_from<I&, const I2&>
//     constexpr counted_iterator& operator=(const counted_iterator<I2>& x);

#include <uscl/std/iterator>

#include "test_iterators.h"
#include "test_macros.h"

class AssignableFromIter
{
  int* it_;

public:
  typedef cuda::std::input_iterator_tag iterator_category;
  typedef int value_type;
  typedef typename cuda::std::iterator_traits<int*>::difference_type difference_type;
  typedef int* pointer;
  typedef int& reference;

  __host__ __device__ friend constexpr int* base(const AssignableFromIter& i)
  {
    return i.it_;
  }

  AssignableFromIter() = default;
  __host__ __device__ explicit constexpr AssignableFromIter(int* it)
      : it_(it)
  {}
  __host__ __device__ constexpr AssignableFromIter(const forward_iterator<int*>& it)
      : it_(base(it))
  {}

  __host__ __device__ constexpr AssignableFromIter& operator=(const forward_iterator<int*>& other)
  {
    it_ = base(other);
    return *this;
  }

  __host__ __device__ constexpr reference operator*() const
  {
    return *it_;
  }

  __host__ __device__ constexpr AssignableFromIter& operator++()
  {
    ++it_;
    return *this;
  }
  __host__ __device__ constexpr AssignableFromIter operator++(int)
  {
    AssignableFromIter tmp(*this);
    ++(*this);
    return tmp;
  }
};

struct InputOrOutputArchetype
{
  using difference_type = int;

  int* ptr;

  __host__ __device__ int operator*()
  {
    return *ptr;
  }
  __host__ __device__ void operator++(int)
  {
    ++ptr;
  }
  __host__ __device__ InputOrOutputArchetype& operator++()
  {
    ++ptr;
    return *this;
  }
};

__host__ __device__ constexpr bool test()
{
  int buffer[8] = {1, 2, 3, 4, 5, 6, 7, 8};

  {
    static_assert(cuda::std::is_assignable_v<cuda::std::counted_iterator<forward_iterator<int*>>,
                                             cuda::std::counted_iterator<forward_iterator<int*>>>);
    static_assert(!cuda::std::is_assignable_v<cuda::std::counted_iterator<forward_iterator<int*>>,
                                              cuda::std::counted_iterator<random_access_iterator<int*>>>);
  }

  {
    cuda::std::counted_iterator iter1(AssignableFromIter{buffer}, 8);
    cuda::std::counted_iterator iter2(forward_iterator<int*>{buffer + 2}, 6);
    assert(base(iter1.base()) == buffer);
    assert(iter1.count() == 8);
    cuda::std::counted_iterator<AssignableFromIter>& result = (iter1 = iter2);
    assert(&result == &iter1);
    assert(base(iter1.base()) == buffer + 2);
    assert(iter1.count() == 6);

    static_assert(cuda::std::is_same_v<decltype(iter1 = iter2), cuda::std::counted_iterator<AssignableFromIter>&>);
  }
  {
    cuda::std::counted_iterator iter1(AssignableFromIter{buffer}, 8);
    const cuda::std::counted_iterator iter2(forward_iterator<int*>{buffer + 2}, 6);
    assert(base(iter1.base()) == buffer);
    assert(iter1.count() == 8);
    cuda::std::counted_iterator<AssignableFromIter>& result = (iter1 = iter2);
    assert(&result == &iter1);
    assert(base(iter1.base()) == buffer + 2);
    assert(iter1.count() == 6);

    static_assert(cuda::std::is_same_v<decltype(iter1 = iter2), cuda::std::counted_iterator<AssignableFromIter>&>);
  }

  {
    cuda::std::counted_iterator iter1(InputOrOutputArchetype{buffer}, 8);
    cuda::std::counted_iterator iter2(InputOrOutputArchetype{buffer + 2}, 6);
    assert(iter1.base().ptr == buffer);
    assert(iter1.count() == 8);
    cuda::std::counted_iterator<InputOrOutputArchetype>& result = (iter1 = iter2);
    assert(&result == &iter1);
    assert(iter1.base().ptr == buffer + 2);
    assert(iter1.count() == 6);

    static_assert(cuda::std::is_same_v<decltype(iter1 = iter2), cuda::std::counted_iterator<InputOrOutputArchetype>&>);
  }
  {
    cuda::std::counted_iterator iter1(InputOrOutputArchetype{buffer}, 8);
    const cuda::std::counted_iterator iter2(InputOrOutputArchetype{buffer + 2}, 6);
    assert(iter1.base().ptr == buffer);
    assert(iter1.count() == 8);
    cuda::std::counted_iterator<InputOrOutputArchetype>& result = (iter1 = iter2);
    assert(&result == &iter1);
    assert(iter1.base().ptr == buffer + 2);
    assert(iter1.count() == 6);

    static_assert(cuda::std::is_same_v<decltype(iter1 = iter2), cuda::std::counted_iterator<InputOrOutputArchetype>&>);
  }

  return true;
}

int main(int, char**)
{
  test();
  static_assert(test());

  return 0;
}
