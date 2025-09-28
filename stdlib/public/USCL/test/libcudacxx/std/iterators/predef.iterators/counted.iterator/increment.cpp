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

// constexpr counted_iterator& operator++();
// decltype(auto) operator++(int);
// constexpr counted_iterator operator++(int)
//   requires forward_iterator<I>;

#include <uscl/std/iterator>

#include "test_iterators.h"
#include "test_macros.h"

#if TEST_HAS_EXCEPTIONS()
template <class It>
class ThrowsOnInc
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

  ThrowsOnInc() = default;
  __host__ __device__ explicit constexpr ThrowsOnInc(It it)
      : it_(it)
  {}

  __host__ __device__ constexpr reference operator*() const
  {
    return *it_;
  }

  __host__ __device__ constexpr ThrowsOnInc& operator++()
  {
    throw 42;
  }
  __host__ __device__ constexpr ThrowsOnInc operator++(int)
  {
    throw 42;
  }
};
#endif // TEST_HAS_EXCEPTIONS()

struct InputOrOutputArchetype
{
  using difference_type = int;

  int* ptr;

  __host__ __device__ constexpr int operator*() const
  {
    return *ptr;
  }
  __host__ __device__ constexpr void operator++(int)
  {
    ++ptr;
  }
  __host__ __device__ constexpr InputOrOutputArchetype& operator++()
  {
    ++ptr;
    return *this;
  }
};

template <class Iter>
_CCCL_CONCEPT PlusEnabled = _CCCL_REQUIRES_EXPR((Iter), Iter& iter)((iter++), (++iter));

__host__ __device__ constexpr bool test()
{
  int buffer[8] = {1, 2, 3, 4, 5, 6, 7, 8};

  {
    using Counted = cuda::std::counted_iterator<forward_iterator<int*>>;
    cuda::std::counted_iterator iter(forward_iterator<int*>{buffer}, 8);

    assert(iter++ == Counted(forward_iterator<int*>{buffer}, 8));
    assert(++iter == Counted(forward_iterator<int*>{buffer + 2}, 6));

    static_assert(cuda::std::is_same_v<decltype(iter++), Counted>);
    static_assert(cuda::std::is_same_v<decltype(++iter), Counted&>);
  }
  {
    using Counted = cuda::std::counted_iterator<random_access_iterator<int*>>;
    cuda::std::counted_iterator iter(random_access_iterator<int*>{buffer}, 8);

    assert(iter++ == Counted(random_access_iterator<int*>{buffer}, 8));
    assert(++iter == Counted(random_access_iterator<int*>{buffer + 2}, 6));

    static_assert(cuda::std::is_same_v<decltype(iter++), Counted>);
    static_assert(cuda::std::is_same_v<decltype(++iter), Counted&>);
  }

  {
    static_assert(PlusEnabled<cuda::std::counted_iterator<random_access_iterator<int*>>>);
    static_assert(!PlusEnabled<const cuda::std::counted_iterator<random_access_iterator<int*>>>);
  }

  return true;
}

int main(int, char**)
{
  test();
  static_assert(test());

  int buffer[8] = {1, 2, 3, 4, 5, 6, 7, 8};

  {
    using Counted = cuda::std::counted_iterator<InputOrOutputArchetype>;
    cuda::std::counted_iterator iter(InputOrOutputArchetype{buffer}, 8);

    iter++;
    assert((++iter).base().ptr == buffer + 2);

    static_assert(cuda::std::is_same_v<decltype(iter++), void>);
    static_assert(cuda::std::is_same_v<decltype(++iter), Counted&>);
  }
  {
    using Counted = cuda::std::counted_iterator<cpp20_input_iterator<int*>>;
    cuda::std::counted_iterator iter(cpp20_input_iterator<int*>{buffer}, 8);

    iter++;
    assert(++iter == Counted(cpp20_input_iterator<int*>{buffer + 2}, 6));

    static_assert(cuda::std::is_same_v<decltype(iter++), void>);
    static_assert(cuda::std::is_same_v<decltype(++iter), Counted&>);
  }
#if TEST_HAS_EXCEPTIONS()
  {
    using Counted = cuda::std::counted_iterator<ThrowsOnInc<int*>>;
    cuda::std::counted_iterator iter(ThrowsOnInc<int*>{buffer}, 8);
    try
    {
      (void) iter++;
      assert(false);
    }
    catch (int x)
    {
      assert(x == 42);
      assert(iter.count() == 8);
    }

    static_assert(cuda::std::is_same_v<decltype(iter++), ThrowsOnInc<int*>>);
    static_assert(cuda::std::is_same_v<decltype(++iter), Counted&>);
  }
#endif // TEST_HAS_EXCEPTIONS()

  return 0;
}
