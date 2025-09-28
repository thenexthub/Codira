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

// Check that the nested types of cuda::std::allocator are provided:

// template <class T>
// class allocator
// {
// public:
//     typedef size_t    size_type;
//     typedef ptrdiff_t difference_type;
//     typedef T         value_type;
//
//     typedef T*        pointer;           // deprecated in C++17, removed in C++20
//     typedef T const*  const_pointer;     // deprecated in C++17, removed in C++20
//     typedef T&        reference;         // deprecated in C++17, removed in C++20
//     typedef T const&  const_reference;   // deprecated in C++17, removed in C++20
//     template< class U > struct rebind { typedef allocator<U> other; }; // deprecated in C++17, removed in C++20
//
//     typedef true_type propagate_on_container_move_assignment;
//     typedef true_type is_always_equal;
// ...
// };

// ADDITIONAL_COMPILE_DEFINITIONS: _LIBCUDACXX_DISABLE_DEPRECATION_WARNINGS

#include <uscl/std/__memory_>
#include <uscl/std/cstddef>
#include <uscl/std/type_traits>

#include "test_macros.h"

struct U;

template <typename T>
__host__ __device__ void test()
{
  typedef cuda::std::allocator<T> Alloc;
  static_assert((cuda::std::is_same<typename Alloc::size_type, cuda::std::size_t>::value), "");
  static_assert((cuda::std::is_same<typename Alloc::difference_type, cuda::std::ptrdiff_t>::value), "");
  static_assert((cuda::std::is_same<typename Alloc::value_type, T>::value), "");
  static_assert(
    (cuda::std::is_same<typename Alloc::propagate_on_container_move_assignment, cuda::std::true_type>::value), "");
  static_assert((cuda::std::is_same<typename Alloc::is_always_equal, cuda::std::true_type>::value), "");

#if TEST_STD_VER <= 2017
  static_assert((cuda::std::is_same<typename Alloc::pointer, T*>::value), "");
  static_assert((cuda::std::is_same<typename Alloc::const_pointer, T const*>::value), "");
  static_assert((cuda::std::is_same<typename Alloc::reference, T&>::value), "");
  static_assert((cuda::std::is_same<typename Alloc::const_reference, T const&>::value), "");
  static_assert((cuda::std::is_same<typename Alloc::template rebind<U>::other, cuda::std::allocator<U>>::value), "");
#endif // TEST_STD_VER <= 2017
}

int main(int, char**)
{
  test<char>();
#ifdef _LIBCUDACXX_VERSION
  test<char const>(); // extension
#endif
  return 0;
}
