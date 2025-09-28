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
//     static constexpr allocator_type
//         select_on_container_copy_construction(const allocator_type& a);
//     ...
// };

#include <uscl/std/__memory_>
#include <uscl/std/cassert>
#include <uscl/std/type_traits>

#include "incomplete_type_helper.h"
#include "test_macros.h"

template <class T>
struct A
{
  typedef T value_type;
  int id;
  __host__ __device__ TEST_CONSTEXPR_CXX20 explicit A(int i = 0)
      : id(i)
  {}
};

template <class T>
struct B
{
  typedef T value_type;

  int id;
  __host__ __device__ TEST_CONSTEXPR_CXX20 explicit B(int i = 0)
      : id(i)
  {}

  __host__ __device__ TEST_CONSTEXPR_CXX20 B select_on_container_copy_construction() const
  {
    return B(100);
  }
};

__host__ __device__ TEST_CONSTEXPR_CXX20 bool test()
{
  {
    A<int> a;
    assert(cuda::std::allocator_traits<A<int>>::select_on_container_copy_construction(a).id == 0);
  }
  {
    const A<int> a(0);
    assert(cuda::std::allocator_traits<A<int>>::select_on_container_copy_construction(a).id == 0);
  }
  {
    typedef IncompleteHolder* VT;
    typedef A<VT> Alloc;
    Alloc a;
    assert(cuda::std::allocator_traits<Alloc>::select_on_container_copy_construction(a).id == 0);
  }
  {
    B<int> b;
    assert(cuda::std::allocator_traits<B<int>>::select_on_container_copy_construction(b).id == 100);
  }
  {
    const B<int> b(0);
    assert(cuda::std::allocator_traits<B<int>>::select_on_container_copy_construction(b).id == 100);
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
