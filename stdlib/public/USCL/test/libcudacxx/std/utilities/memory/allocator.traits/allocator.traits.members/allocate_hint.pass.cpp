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
//     static constexpr pointer allocate(allocator_type& a, size_type n, const_void_pointer hint);
//     ...
// };

#include <uscl/std/__memory_>
#include <uscl/std/cassert>
#include <uscl/std/cstdint>

#include "incomplete_type_helper.h"
#include "test_macros.h"

template <class T>
struct A
{
  typedef T value_type;

  __host__ __device__ TEST_CONSTEXPR_CXX20 A() {}

  __host__ __device__ TEST_CONSTEXPR_CXX20 value_type* allocate(cuda::std::size_t n)
  {
    assert(n == 10);
    return &storage;
  }

  value_type storage;
};

template <class T>
struct B
{
  typedef T value_type;

  __host__ __device__ TEST_CONSTEXPR_CXX20 value_type* allocate(cuda::std::size_t n, const void* p)
  {
    assert(n == 11);
    assert(p == nullptr);
    return &storage;
  }

  value_type storage;
};

__host__ __device__ TEST_CONSTEXPR_CXX20 bool test()
{
  {
    A<int> a;
    assert(cuda::std::allocator_traits<A<int>>::allocate(a, 10, nullptr) == &a.storage);
  }
  {
    typedef A<IncompleteHolder*> Alloc;
    Alloc a;
    assert(cuda::std::allocator_traits<Alloc>::allocate(a, 10, nullptr) == &a.storage);
  }
  {
    B<int> b;
    assert(cuda::std::allocator_traits<B<int>>::allocate(b, 11, nullptr) == &b.storage);
  }
  {
    typedef B<IncompleteHolder*> Alloc;
    Alloc b;
    assert(cuda::std::allocator_traits<Alloc>::allocate(b, 11, nullptr) == &b.storage);
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
