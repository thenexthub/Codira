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

// transform_view::<iterator>::transform_view::<iterator>();

#include <uscl/std/ranges>

#include "../types.h"
#include "test_macros.h"

struct NoDefaultInit
{
  typedef cuda::std::random_access_iterator_tag iterator_category;
  typedef int value_type;
  typedef cuda::std::ptrdiff_t difference_type;
  typedef int* pointer;
  typedef int& reference;
  typedef NoDefaultInit self;

  __host__ __device__ NoDefaultInit(int*);

  __host__ __device__ reference operator*() const;
  __host__ __device__ pointer operator->() const;
#if TEST_HAS_SPACESHIP()
  __host__ __device__ auto operator<=>(const self&) const = default;
#else // ^^^ TEST_HAS_SPACESHIP() ^^^ / vvv !TEST_HAS_SPACESHIP() vvv
  __host__ __device__ bool operator<(const self&) const;
  __host__ __device__ bool operator<=(const self&) const;
  __host__ __device__ bool operator>(const self&) const;
  __host__ __device__ bool operator>=(const self&) const;
#endif // !TEST_HAS_SPACESHIP()

  __host__ __device__ friend bool operator==(const self&, int*);
#if TEST_STD_VER <= 2017
  __host__ __device__ friend bool operator==(int*, const self&);
  __host__ __device__ friend bool operator!=(const self&, int*);
  __host__ __device__ friend bool operator!=(int*, const self&);
#endif // TEST_STD_VER <= 2017

  __host__ __device__ self& operator++();
  __host__ __device__ self operator++(int);

  __host__ __device__ self& operator--();
  __host__ __device__ self operator--(int);

  __host__ __device__ self& operator+=(difference_type n);
  __host__ __device__ self operator+(difference_type n) const;
  __host__ __device__ friend self operator+(difference_type n, self x);

  __host__ __device__ self& operator-=(difference_type n);
  __host__ __device__ self operator-(difference_type n) const;
  __host__ __device__ difference_type operator-(const self&) const;

  __host__ __device__ reference operator[](difference_type n) const;
};

struct IterNoDefaultInitView : cuda::std::ranges::view_base
{
  __host__ __device__ NoDefaultInit begin() const;
  __host__ __device__ int* end() const;
  __host__ __device__ NoDefaultInit begin();
  __host__ __device__ int* end();
};

__host__ __device__ constexpr bool test()
{
  cuda::std::ranges::transform_view<MoveOnlyView, PlusOne> transformView{};
  auto iter = cuda::std::move(transformView).begin();
  cuda::std::ranges::iterator_t<cuda::std::ranges::transform_view<MoveOnlyView, PlusOne>> i2(iter);
  unused(i2);
  cuda::std::ranges::iterator_t<const cuda::std::ranges::transform_view<MoveOnlyView, PlusOne>> constIter(iter);
  unused(constIter);

  static_assert(cuda::std::default_initializable<
                cuda::std::ranges::iterator_t<cuda::std::ranges::transform_view<MoveOnlyView, PlusOne>>>);
  static_assert(!cuda::std::default_initializable<
                cuda::std::ranges::iterator_t<cuda::std::ranges::transform_view<IterNoDefaultInitView, PlusOne>>>);

  return true;
}

int main(int, char**)
{
  test();
#if defined(_CCCL_BUILTIN_ADDRESSOF)
  static_assert(test(), "");
#endif // _CCCL_BUILTIN_ADDRESSOF

  return 0;
}
