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

// unique_ptr

// template <class T1, class D1, class T2, class D2>
//   bool
//   operator==(const unique_ptr<T1, D1>& x, const unique_ptr<T2, D2>& y);

// template <class T1, class D1, class T2, class D2>
//   bool
//   operator!=(const unique_ptr<T1, D1>& x, const unique_ptr<T2, D2>& y);

// template <class T1, class D1, class T2, class D2>
//   bool
//   operator< (const unique_ptr<T1, D1>& x, const unique_ptr<T2, D2>& y);

// template <class T1, class D1, class T2, class D2>
//   bool
//   operator> (const unique_ptr<T1, D1>& x, const unique_ptr<T2, D2>& y);

// template <class T1, class D1, class T2, class D2>
//   bool
//   operator<=(const unique_ptr<T1, D1>& x, const unique_ptr<T2, D2>& y);

// template <class T1, class D1, class T2, class D2>
//   bool
//   operator>=(const unique_ptr<T1, D1>& x, const unique_ptr<T2, D2>& y);

// template<class T1, class D1, class T2, class D2>
//   requires three_way_comparable_with<typename unique_ptr<T1, D1>::pointer,
//                                      typename unique_ptr<T2, D2>::pointer>
//   compare_three_way_result_t<typename unique_ptr<T1, D1>::pointer,
//                              typename unique_ptr<T2, D2>::pointer>
//     operator<=>(const unique_ptr<T1, D1>& x, const unique_ptr<T2, D2>& y);

#include <uscl/std/__memory_>
#include <uscl/std/cassert>

#include "deleter_types.h"
#include "test_comparisons.h"
#include "test_macros.h"
#include "unique_ptr_test_helper.h"

__host__ __device__ TEST_CONSTEXPR_CXX23 bool test()
{
  AssertComparisonsReturnBool<cuda::std::unique_ptr<int>>();
#if TEST_STD_VER >= 2020 && _LIBCUDACXX_HAS_SPACESHIP_OPERATOR()
  AssertOrderReturn<cuda::std::strong_ordering, cuda::std::unique_ptr<int>>();
#endif // TEST_STD_VER >= 2020 && _LIBCUDACXX_HAS_SPACESHIP_OPERATOR()

  // Pointers of same type
  {
    A* ptr1 = new A;
    A* ptr2 = new A;
    const cuda::std::unique_ptr<A, Deleter<A>> p1(ptr1);
    const cuda::std::unique_ptr<A, Deleter<A>> p2(ptr2);

    assert(!(p1 == p2));
    assert(p1 != p2);
    if (!TEST_IS_CONSTANT_EVALUATED_CXX23())
    {
      assert((p1 < p2) == (ptr1 < ptr2));
      assert((p1 <= p2) == (ptr1 <= ptr2));
      assert((p1 > p2) == (ptr1 > ptr2));
      assert((p1 >= p2) == (ptr1 >= ptr2));
#if TEST_STD_VER >= 2020 && _LIBCUDACXX_HAS_SPACESHIP_OPERATOR()
      assert((p1 <=> p2) != cuda::std::strong_ordering::equal);
      assert((p1 <=> p2) == (ptr1 <=> ptr2));
#endif // TEST_STD_VER >= 2020 && _LIBCUDACXX_HAS_SPACESHIP_OPERATOR()
    }
  }
  // Pointers of different type
  {
    A* ptr1 = new A;
    B* ptr2 = new B;
    const cuda::std::unique_ptr<A, Deleter<A>> p1(ptr1);
    const cuda::std::unique_ptr<B, Deleter<B>> p2(ptr2);
    assert(!(p1 == p2));
    assert(p1 != p2);
    if (!TEST_IS_CONSTANT_EVALUATED_CXX23())
    {
      assert((p1 < p2) == (ptr1 < ptr2));
      assert((p1 <= p2) == (ptr1 <= ptr2));
      assert((p1 > p2) == (ptr1 > ptr2));
      assert((p1 >= p2) == (ptr1 >= ptr2));
#if TEST_STD_VER >= 2020 && _LIBCUDACXX_HAS_SPACESHIP_OPERATOR()
      assert((p1 <=> p2) != cuda::std::strong_ordering::equal);
      assert((p1 <=> p2) == (ptr1 <=> ptr2));
#endif // TEST_STD_VER >= 2020 && _LIBCUDACXX_HAS_SPACESHIP_OPERATOR()
    }
  }
  // Pointers of same array type
  {
    A* ptr1 = new A[3];
    A* ptr2 = new A[3];
    const cuda::std::unique_ptr<A[], Deleter<A[]>> p1(ptr1);
    const cuda::std::unique_ptr<A[], Deleter<A[]>> p2(ptr2);
    assert(!(p1 == p2));
    assert(p1 != p2);
    if (!TEST_IS_CONSTANT_EVALUATED_CXX23())
    {
      assert((p1 < p2) == (ptr1 < ptr2));
      assert((p1 <= p2) == (ptr1 <= ptr2));
      assert((p1 > p2) == (ptr1 > ptr2));
      assert((p1 >= p2) == (ptr1 >= ptr2));
#if TEST_STD_VER >= 2020 && _LIBCUDACXX_HAS_SPACESHIP_OPERATOR()
      assert((p1 <=> p2) != cuda::std::strong_ordering::equal);
      assert((p1 <=> p2) == (ptr1 <=> ptr2));
#endif // TEST_STD_VER >= 2020 && _LIBCUDACXX_HAS_SPACESHIP_OPERATOR()
    }
  }
  // Pointers of different array types
  {
    A* ptr1 = new A[3];
    B* ptr2 = new B[3];
    const cuda::std::unique_ptr<A[], Deleter<A[]>> p1(ptr1);
    const cuda::std::unique_ptr<B[], Deleter<B[]>> p2(ptr2);
    assert(!(p1 == p2));
    assert(p1 != p2);
    if (!TEST_IS_CONSTANT_EVALUATED_CXX23())
    {
      assert((p1 < p2) == (ptr1 < ptr2));
      assert((p1 <= p2) == (ptr1 <= ptr2));
      assert((p1 > p2) == (ptr1 > ptr2));
      assert((p1 >= p2) == (ptr1 >= ptr2));
#if TEST_STD_VER >= 2020 && _LIBCUDACXX_HAS_SPACESHIP_OPERATOR()
      assert((p1 <=> p2) != cuda::std::strong_ordering::equal);
      assert((p1 <=> p2) == (ptr1 <=> ptr2));
#endif // TEST_STD_VER >= 2020 && _LIBCUDACXX_HAS_SPACESHIP_OPERATOR()
    }
  }
  // Default-constructed pointers of same type
  {
    const cuda::std::unique_ptr<A, Deleter<A>> p1;
    const cuda::std::unique_ptr<A, Deleter<A>> p2;
    assert(p1 == p2);
#if TEST_STD_VER >= 2020 && _LIBCUDACXX_HAS_SPACESHIP_OPERATOR()
    if (!TEST_IS_CONSTANT_EVALUATED_CXX23())
    {
      assert((p1 <=> p2) == cuda::std::strong_ordering::equal);
    }
#endif // TEST_STD_VER >= 2020 && _LIBCUDACXX_HAS_SPACESHIP_OPERATOR()
  }
  // Default-constructed pointers of different type
  {
    const cuda::std::unique_ptr<A, Deleter<A>> p1;
    const cuda::std::unique_ptr<B, Deleter<B>> p2;
    assert(p1 == p2);
#if TEST_STD_VER >= 2020 && _LIBCUDACXX_HAS_SPACESHIP_OPERATOR()
    if (!TEST_IS_CONSTANT_EVALUATED_CXX23())
    {
      assert((p1 <=> p2) == cuda::std::strong_ordering::equal);
    }
#endif // TEST_STD_VER >= 2020 && _LIBCUDACXX_HAS_SPACESHIP_OPERATOR()
  }

  return true;
}

int main(int, char**)
{
  test();
#if TEST_STD_VER >= 2023
  static_assert(test());
#endif // TEST_STD_VER >= 2023

  return 0;
}
