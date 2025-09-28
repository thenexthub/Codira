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

// transform_iterator::transform_iterator();

#include <uscl/iterator>
#include <uscl/std/cassert>
#include <uscl/std/concepts>

#include "test_iterators.h"
#include "test_macros.h"
#include "types.h"

struct NoDefaultInitIter
{
  int* ptr_;
  using iterator_category = cuda::std::random_access_iterator_tag;
  using value_type        = int;
  using difference_type   = cuda::std::ptrdiff_t;
  using pointer           = int*;
  using reference         = int&;
  using self              = NoDefaultInitIter;

  __host__ __device__ constexpr NoDefaultInitIter(int* ptr)
      : ptr_(ptr)
  {}

  __host__ __device__ constexpr reference operator*() const;
  __host__ __device__ constexpr pointer operator->() const;
#if TEST_HAS_SPACESHIP()
  __host__ __device__ constexpr auto operator<=>(const self&) const = default;
#else // ^^^ TEST_HAS_SPACESHIP() ^^^ / vvv !TEST_HAS_SPACESHIP() vvv
  __host__ __device__ constexpr bool operator<(const self&) const;
  __host__ __device__ constexpr bool operator<=(const self&) const;
  __host__ __device__ constexpr bool operator>(const self&) const;
  __host__ __device__ constexpr bool operator>=(const self&) const;
#endif // !TEST_HAS_SPACESHIP()

  __host__ __device__ constexpr friend bool operator==(const self& lhs, const self& rhs)
  {
    return lhs.ptr_ == rhs.ptr_;
  }
#if TEST_STD_VER <= 2017
  __host__ __device__ constexpr friend bool operator!=(const self& lhs, const self& rhs)
  {
    return lhs.ptr_ != rhs.ptr_;
  }
#endif // TEST_STD_VER <= 2017

  __host__ __device__ constexpr self& operator++();
  __host__ __device__ constexpr self operator++(int);

  __host__ __device__ constexpr self& operator--();
  __host__ __device__ constexpr self operator--(int);

  __host__ __device__ constexpr self& operator+=(difference_type n);
  __host__ __device__ constexpr self operator+(difference_type n) const;
  __host__ __device__ constexpr friend self operator+(difference_type n, self x);

  __host__ __device__ constexpr self& operator-=(difference_type n);
  __host__ __device__ constexpr self operator-(difference_type n) const;
  __host__ __device__ constexpr difference_type operator-(const self&) const;

  __host__ __device__ constexpr reference operator[](difference_type n) const;
};

struct NoDefaultInitFunc
{
  int val_;

  __host__ __device__ constexpr NoDefaultInitFunc(int val)
      : val_(val)
  {}

  __host__ __device__ constexpr int operator()(int x) const
  {
    return x * val_;
  }
};

template <class Iter, class Fn>
__host__ __device__ constexpr void test(Fn fun)
{
  int buffer[8] = {0, 1, 2, 3, 4, 5, 6, 7};

  { // default initialization
    constexpr bool can_default_init = cuda::std::default_initializable<Iter> && cuda::std::default_initializable<Fn>;
    static_assert(cuda::std::default_initializable<cuda::transform_iterator<Iter, Fn>> == can_default_init);
    if constexpr (can_default_init)
    {
      [[maybe_unused]] cuda::transform_iterator<Iter, Fn> iter{};
    }
  }

  { // construction from iter and functor
    cuda::transform_iterator iter{Iter{buffer}, fun};
    assert(iter.base() == Iter{buffer});
  }
}

__host__ __device__ constexpr bool test()
{
  test<NoDefaultInitIter>(PlusOne{});
  test<random_access_iterator<int*>>(PlusOne{});

  NoDefaultInitFunc func{42};
  test<NoDefaultInitIter>(func);
  test<random_access_iterator<int*>>(func);

  return true;
}

int main(int, char**)
{
  test();
  static_assert(test(), "");

  return 0;
}
