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

// <algorithm>

// template<InputIterator InIter1, InputIterator InIter2, typename OutIter,
//          CopyConstructible Compare>
//   requires OutputIterator<OutIter, InIter1::reference>
//         && OutputIterator<OutIter, InIter2::reference>
//         && Predicate<Compare, InIter1::value_type, InIter2::value_type>
//         && Predicate<Compare, InIter2::value_type, InIter1::value_type>
//   constexpr OutIter       // constexpr after C++17
//   set_intersection(InIter1 first1, InIter1 last1, InIter2 first2, InIter2 last2,
//                    OutIter result, Compare comp);

#include <uscl/std/__algorithm_>
#include <uscl/std/cassert>

#include "../../sortable_helpers.h"
#include "test_iterators.h"
#include "test_macros.h"

template <class T, class Iter1, class Iter2, class OutIter>
__host__ __device__ constexpr void test4()
{
  const T a[] = {11, 33, 31, 41};
  const T b[] = {22, 32, 43, 42, 52};
  {
    T result[20] = {};
    T expected[] = {33, 41};
    OutIter end  = cuda::std::set_intersection(
      Iter1(a), Iter1(a + 4), Iter2(b), Iter2(b + 5), OutIter(result), typename T::Comparator());
    assert(cuda::std::lexicographical_compare(result, base(end), expected, expected + 2, T::less) == 0);
    for (const T* it = base(end); it != result + 20; ++it)
    {
      assert(it->value == 0);
    }
  }
  {
    T result[20] = {};
    T expected[] = {32, 42};
    OutIter end  = cuda::std::set_intersection(
      Iter1(b), Iter1(b + 5), Iter2(a), Iter2(a + 4), OutIter(result), typename T::Comparator());
    assert(cuda::std::lexicographical_compare(result, base(end), expected, expected + 2, T::less) == 0);
    for (const T* it = base(end); it != result + 20; ++it)
    {
      assert(it->value == 0);
    }
  }
}

template <class T, class Iter1, class Iter2>
__host__ __device__ constexpr void test3()
{
  test4<T, Iter1, Iter2, cpp17_output_iterator<T*>>();
  test4<T, Iter1, Iter2, forward_iterator<T*>>();
  test4<T, Iter1, Iter2, bidirectional_iterator<T*>>();
  test4<T, Iter1, Iter2, random_access_iterator<T*>>();
  test4<T, Iter1, Iter2, T*>();
}

template <class T, class Iter1>
__host__ __device__ constexpr void test2()
{
  test3<T, Iter1, cpp17_input_iterator<const T*>>();
  test3<T, Iter1, forward_iterator<const T*>>();
  test3<T, Iter1, bidirectional_iterator<const T*>>();
  test3<T, Iter1, random_access_iterator<const T*>>();
  test3<T, Iter1, const T*>();
}

template <class T>
__host__ __device__ constexpr void test1()
{
  test2<T, cpp17_input_iterator<const T*>>();
  test2<T, forward_iterator<const T*>>();
  test2<T, bidirectional_iterator<const T*>>();
  test2<T, random_access_iterator<const T*>>();
  test2<T, const T*>();
}

__host__ __device__ constexpr bool test()
{
  test1<TrivialSortableWithComp>();
  test1<NonTrivialSortableWithComp>();
  return true;
}

int main(int, char**)
{
  test();
  static_assert(test(), "");

  return 0;
}
