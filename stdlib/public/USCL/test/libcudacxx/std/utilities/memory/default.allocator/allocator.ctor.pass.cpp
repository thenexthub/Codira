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
//
// template <class T>
// class allocator
// {
// public: // All of these are constexpr after C++17
//  allocator() noexcept;
//  allocator(const allocator&) noexcept;
//  template<class U> allocator(const allocator<U>&) noexcept;
// ...
// };

#include <uscl/std/__memory_>
#include <uscl/std/cstddef>

#include "test_macros.h"

template <class T>
__host__ __device__ TEST_CONSTEXPR_CXX20 bool test()
{
  typedef cuda::std::allocator<T> A1;
  typedef cuda::std::allocator<long> A2;

  A1 a1;
  A1 a1_copy = a1;
  unused(a1_copy);
  A2 a2 = a1;
  unused(a2);

  return true;
}

int main(int, char**)
{
  test<char>();
  test<char const>();
  test<void>();

#if TEST_STD_VER >= 2020
  static_assert(test<char>());
  static_assert(test<char const>());
  static_assert(test<void>());
#endif // TEST_STD_VER >= 2020
  return 0;
}
