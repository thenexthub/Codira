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

// allocator:
// constexpr T* allocate(size_type n);

// UNSUPPORTED: c++17

#include <uscl/std/__memory_>
#include <uscl/std/cstddef>

#include "test_macros.h"

template <typename T>
__host__ __device__ constexpr bool test()
{
  typedef cuda::std::allocator<T> A;
  typedef cuda::std::allocator_traits<A> AT;
  A a;
  TEST_IGNORE_NODISCARD a.allocate(AT::max_size(a) + 1); // just barely too large
  TEST_IGNORE_NODISCARD a.allocate(AT::max_size(a) * 2); // significantly too large
  TEST_IGNORE_NODISCARD a.allocate(((cuda::std::size_t) -1) / sizeof(T) + 1); // multiply will overflow
  TEST_IGNORE_NODISCARD a.allocate((cuda::std::size_t) -1); // way too large

  return true;
}

__host__ __device__ void f()
{
  static_assert(test<double>()); // expected-error-re {{{{(static_assert|static assertion)}} expression is not an
                                 // integral constant expression}}
  static_assert(test<const double>()); // expected-error-re {{{{(static_assert|static assertion)}} expression is
                                       // not an integral constant expression}}
}

int main(int, char**)
{
  return 0;
}
