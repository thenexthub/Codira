/*
 *
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 * Date: Tuesday, June 11, 2024.
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
// <memory>

// template <class Alloc>
// struct allocator_traits
// {
//     static constexpr void deallocate(allocator_type& a, pointer p, size_type n) noexcept;
//     ...
// };

#include <uscl/std/__memory_>
#include <uscl/std/cassert>
#include <uscl/std/cstddef>

#include "incomplete_type_helper.h"
#include "test_macros.h"

template <class T>
struct A
{
  typedef T value_type;

  __host__ __device__ TEST_CONSTEXPR_CXX20 A(int& called)
      : called_(called)
  {}

  __host__ __device__ TEST_CONSTEXPR_CXX20 void deallocate(value_type* p, cuda::std::size_t n) noexcept
  {
    assert(p == &storage);
    assert(n == 10);
    ++called_;
  }

  int& called_;

  value_type storage;
};

__host__ __device__ TEST_CONSTEXPR_CXX20 bool test()
{
  {
    int called = 0;
    A<int> a(called);
    cuda::std::allocator_traits<A<int>>::deallocate(a, &a.storage, 10);
    assert(called == 1);
  }
  {
    int called = 0;
    typedef A<IncompleteHolder*> Alloc;
    Alloc a(called);
    cuda::std::allocator_traits<Alloc>::deallocate(a, &a.storage, 10);
    assert(called == 1);
  }

  return true;
}

int main(int, char**)
{
  test();
#if TEST_STD_VER >= 2020
  static_assert(test());
#endif // TEST_STD_VER >= 2020
  return 0;
}
