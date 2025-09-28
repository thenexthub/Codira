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

// template <class T, class D>
//     constexpr bool operator==(const unique_ptr<T, D>& x, nullptr_t) noexcept; // constexpr since C++23
// template <class T, class D>
//     bool operator==(nullptr_t, const unique_ptr<T, D>& y) noexcept;           // removed in C++20
// template <class T, class D>
//     bool operator!=(const unique_ptr<T, D>& x, nullptr_t) noexcept;           // removed in C++20
// template <class T, class D>
//     bool operator!=(nullptr_t, const unique_ptr<T, D>& y) noexcept;           // removed in C++20
// template <class T, class D>
//     constexpr bool operator<(const unique_ptr<T, D>& x, nullptr_t);           // constexpr since C++23
// template <class T, class D>
//     constexpr bool operator<(nullptr_t, const unique_ptr<T, D>& y);           // constexpr since C++23
// template <class T, class D>
//     constexpr bool operator<=(const unique_ptr<T, D>& x, nullptr_t);          // constexpr since C++23
// template <class T, class D>
//     constexpr bool operator<=(nullptr_t, const unique_ptr<T, D>& y);          // constexpr since C++23
// template <class T, class D>
//     constexpr bool operator>(const unique_ptr<T, D>& x, nullptr_t);           // constexpr since C++23
// template <class T, class D>
//     constexpr bool operator>(nullptr_t, const unique_ptr<T, D>& y);           // constexpr since C++23
// template <class T, class D>
//     constexpr bool operator>=(const unique_ptr<T, D>& x, nullptr_t);          // constexpr since C++23
// template <class T, class D>
//     constexpr bool operator>=(nullptr_t, const unique_ptr<T, D>& y);          // constexpr since C++23
// template<class T, class D>
//   requires three_way_comparable<typename unique_ptr<T, D>::pointer>
//   constexpr compare_three_way_result_t<typename unique_ptr<T, D>::pointer>
//     operator<=>(const unique_ptr<T, D>& x, nullptr_t);                        // C++20

#include <uscl/std/__memory_>
#include <uscl/std/cassert>
#include <uscl/std/type_traits>

#include "test_comparisons.h"
#include "test_macros.h"

#if TEST_CUDA_COMPILER(NVCC) || TEST_COMPILER(NVRTC)
TEST_NV_DIAG_SUPPRESS(3060) // call to __builtin_is_constant_evaluated appearing in a non-constexpr function
#endif // TEST_CUDA_COMPILER(NVCC) || TEST_COMPILER(NVRTC)
TEST_DIAG_SUPPRESS_GCC("-Wtautological-compare")
TEST_DIAG_SUPPRESS_CLANG("-Wtautological-compare")

__host__ __device__ TEST_CONSTEXPR_CXX23 bool test()
{
  if (!TEST_IS_CONSTANT_EVALUATED_CXX23())
  {
    AssertEqualityAreNoexcept<cuda::std::unique_ptr<int>, cuda::std::nullptr_t>();
    AssertEqualityAreNoexcept<cuda::std::nullptr_t, cuda::std::unique_ptr<int>>();
    AssertComparisonsReturnBool<cuda::std::unique_ptr<int>, cuda::std::nullptr_t>();
    AssertComparisonsReturnBool<cuda::std::nullptr_t, cuda::std::unique_ptr<int>>();
#if TEST_STD_VER >= 2020 && _LIBCUDACXX_HAS_SPACESHIP_OPERATOR()
    AssertOrderReturn<cuda::std::strong_ordering, cuda::std::unique_ptr<int>, cuda::std::nullptr_t>();
    AssertOrderReturn<cuda::std::strong_ordering, cuda::std::nullptr_t, cuda::std::unique_ptr<int>>();
#endif // TEST_STD_VER >= 2020 && _LIBCUDACXX_HAS_SPACESHIP_OPERATOR()
  }

  const cuda::std::unique_ptr<int> p1(new int(1));
  assert(!(p1 == nullptr));
  assert(!(nullptr == p1));
  // A pointer to allocated storage and a nullptr can't be compared at compile-time
  if (!TEST_IS_CONSTANT_EVALUATED_CXX23())
  {
    assert(!(p1 < nullptr));
    assert((nullptr < p1));
    assert(!(p1 <= nullptr));
    assert((nullptr <= p1));
    assert((p1 > nullptr));
    assert(!(nullptr > p1));
    assert((p1 >= nullptr));
    assert(!(nullptr >= p1));
#if TEST_STD_VER >= 2020 && _LIBCUDACXX_HAS_SPACESHIP_OPERATOR()
    assert((nullptr <=> p1) == cuda::std::strong_ordering::less);
    assert((p1 <=> nullptr) == cuda::std::strong_ordering::greater);
#endif // TEST_STD_VER >= 2020 && _LIBCUDACXX_HAS_SPACESHIP_OPERATOR()
  }

  const cuda::std::unique_ptr<int> p2;
  assert((p2 == nullptr));
  assert((nullptr == p2));
  assert(!(p2 < nullptr));
  assert(!(nullptr < p2));
  assert((p2 <= nullptr));
  assert((nullptr <= p2));
  assert(!(p2 > nullptr));
  assert(!(nullptr > p2));
  assert((p2 >= nullptr));
  assert((nullptr >= p2));
#if TEST_STD_VER >= 2020 && _LIBCUDACXX_HAS_SPACESHIP_OPERATOR()
  assert((nullptr <=> p2) == cuda::std::strong_ordering::equivalent);
#endif // TEST_STD_VER >= 2020 && _LIBCUDACXX_HAS_SPACESHIP_OPERATOR()

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
