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
//     static constexpr size_type max_size(const allocator_type& a) noexcept;
//     ...
// };

#include <uscl/std/__memory_>
#include <uscl/std/cassert>
#include <uscl/std/limits>
#include <uscl/std/type_traits>

#include "incomplete_type_helper.h"
#include "test_macros.h"

template <class T>
struct A
{
  typedef T value_type;
};

template <class T>
struct B
{
  typedef T value_type;

  __host__ __device__ TEST_CONSTEXPR_CXX20 cuda::std::size_t max_size() const
  {
    return 100;
  }
};

__host__ __device__ TEST_CONSTEXPR_CXX20 bool test()
{
  {
    B<int> b;
    assert(cuda::std::allocator_traits<B<int>>::max_size(b) == 100);
  }
  {
    const B<int> b = {};
    assert(cuda::std::allocator_traits<B<int>>::max_size(b) == 100);
  }
  {
    typedef IncompleteHolder* VT;
    typedef B<VT> Alloc;
    Alloc a;
    assert(cuda::std::allocator_traits<Alloc>::max_size(a) == 100);
  }
  {
    A<int> a;
    assert(cuda::std::allocator_traits<A<int>>::max_size(a)
           == cuda::std::numeric_limits<cuda::std::size_t>::max() / sizeof(int));
  }
  {
    const A<int> a = {};
    assert(cuda::std::allocator_traits<A<int>>::max_size(a)
           == cuda::std::numeric_limits<cuda::std::size_t>::max() / sizeof(int));
  }
  {
    cuda::std::allocator<int> a;
    static_assert(noexcept(cuda::std::allocator_traits<cuda::std::allocator<int>>::max_size(a)) == true, "");
    unused(a);
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
