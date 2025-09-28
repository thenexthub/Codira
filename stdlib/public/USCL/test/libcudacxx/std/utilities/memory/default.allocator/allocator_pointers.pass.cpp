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

#include <uscl/std/__memory_>
#include <uscl/std/cassert>
#include <uscl/std/type_traits>

#include "test_macros.h"

// <memory>
//
// template <class Alloc>
// struct allocator_traits
// {
//     typedef Alloc                        allocator_type;
//     typedef typename allocator_type::value_type
//                                          value_type;
//
//     typedef Alloc::pointer | value_type* pointer;
//     typedef Alloc::const_pointer
//           | pointer_traits<pointer>::rebind<const value_type>
//                                          const_pointer;
//     typedef Alloc::void_pointer
//           | pointer_traits<pointer>::rebind<void>
//                                          void_pointer;
//     typedef Alloc::const_void_pointer
//           | pointer_traits<pointer>::rebind<const void>
//                                          const_void_pointer;

template <typename Alloc>
__host__ __device__ void test_pointer()
{
  typename cuda::std::allocator_traits<Alloc>::pointer vp;
  typename cuda::std::allocator_traits<Alloc>::const_pointer cvp;

  unused(vp); // Prevent unused warning
  unused(cvp); // Prevent unused warning

  static_assert(cuda::std::is_same<bool, decltype(vp == vp)>::value, "");
  static_assert(cuda::std::is_same<bool, decltype(vp != vp)>::value, "");
  static_assert(cuda::std::is_same<bool, decltype(vp > vp)>::value, "");
  static_assert(cuda::std::is_same<bool, decltype(vp >= vp)>::value, "");
  static_assert(cuda::std::is_same<bool, decltype(vp < vp)>::value, "");
  static_assert(cuda::std::is_same<bool, decltype(vp <= vp)>::value, "");

  static_assert(cuda::std::is_same<bool, decltype(vp == cvp)>::value, "");
  static_assert(cuda::std::is_same<bool, decltype(cvp == vp)>::value, "");
  static_assert(cuda::std::is_same<bool, decltype(vp != cvp)>::value, "");
  static_assert(cuda::std::is_same<bool, decltype(cvp != vp)>::value, "");
  static_assert(cuda::std::is_same<bool, decltype(vp > cvp)>::value, "");
  static_assert(cuda::std::is_same<bool, decltype(cvp > vp)>::value, "");
  static_assert(cuda::std::is_same<bool, decltype(vp >= cvp)>::value, "");
  static_assert(cuda::std::is_same<bool, decltype(cvp >= vp)>::value, "");
  static_assert(cuda::std::is_same<bool, decltype(vp < cvp)>::value, "");
  static_assert(cuda::std::is_same<bool, decltype(cvp < vp)>::value, "");
  static_assert(cuda::std::is_same<bool, decltype(vp <= cvp)>::value, "");
  static_assert(cuda::std::is_same<bool, decltype(cvp <= vp)>::value, "");

  static_assert(cuda::std::is_same<bool, decltype(cvp == cvp)>::value, "");
  static_assert(cuda::std::is_same<bool, decltype(cvp != cvp)>::value, "");
  static_assert(cuda::std::is_same<bool, decltype(cvp > cvp)>::value, "");
  static_assert(cuda::std::is_same<bool, decltype(cvp >= cvp)>::value, "");
  static_assert(cuda::std::is_same<bool, decltype(cvp < cvp)>::value, "");
  static_assert(cuda::std::is_same<bool, decltype(cvp <= cvp)>::value, "");
}

template <typename Alloc>
__host__ __device__ void test_void_pointer()
{
  typename cuda::std::allocator_traits<Alloc>::void_pointer vp;
  typename cuda::std::allocator_traits<Alloc>::const_void_pointer cvp;

  unused(vp); // Prevent unused warning
  unused(cvp); // Prevent unused warning

  static_assert(cuda::std::is_same<bool, decltype(vp == vp)>::value, "");
  static_assert(cuda::std::is_same<bool, decltype(vp != vp)>::value, "");
  static_assert(cuda::std::is_same<bool, decltype(vp > vp)>::value, "");
  static_assert(cuda::std::is_same<bool, decltype(vp >= vp)>::value, "");
  static_assert(cuda::std::is_same<bool, decltype(vp < vp)>::value, "");
  static_assert(cuda::std::is_same<bool, decltype(vp <= vp)>::value, "");

  static_assert(cuda::std::is_same<bool, decltype(vp == cvp)>::value, "");
  static_assert(cuda::std::is_same<bool, decltype(cvp == vp)>::value, "");
  static_assert(cuda::std::is_same<bool, decltype(vp != cvp)>::value, "");
  static_assert(cuda::std::is_same<bool, decltype(cvp != vp)>::value, "");
  static_assert(cuda::std::is_same<bool, decltype(vp > cvp)>::value, "");
  static_assert(cuda::std::is_same<bool, decltype(cvp > vp)>::value, "");
  static_assert(cuda::std::is_same<bool, decltype(vp >= cvp)>::value, "");
  static_assert(cuda::std::is_same<bool, decltype(cvp >= vp)>::value, "");
  static_assert(cuda::std::is_same<bool, decltype(vp < cvp)>::value, "");
  static_assert(cuda::std::is_same<bool, decltype(cvp < vp)>::value, "");
  static_assert(cuda::std::is_same<bool, decltype(vp <= cvp)>::value, "");
  static_assert(cuda::std::is_same<bool, decltype(cvp <= vp)>::value, "");

  static_assert(cuda::std::is_same<bool, decltype(cvp == cvp)>::value, "");
  static_assert(cuda::std::is_same<bool, decltype(cvp != cvp)>::value, "");
  static_assert(cuda::std::is_same<bool, decltype(cvp > cvp)>::value, "");
  static_assert(cuda::std::is_same<bool, decltype(cvp >= cvp)>::value, "");
  static_assert(cuda::std::is_same<bool, decltype(cvp < cvp)>::value, "");
  static_assert(cuda::std::is_same<bool, decltype(cvp <= cvp)>::value, "");
}

struct Foo
{
  int x;
};

int main(int, char**)
{
  test_pointer<cuda::std::allocator<char>>();
  test_pointer<cuda::std::allocator<int>>();
  test_pointer<cuda::std::allocator<Foo>>();

  test_void_pointer<cuda::std::allocator<char>>();
  test_void_pointer<cuda::std::allocator<int>>();
  test_void_pointer<cuda::std::allocator<Foo>>();

  return 0;
}
