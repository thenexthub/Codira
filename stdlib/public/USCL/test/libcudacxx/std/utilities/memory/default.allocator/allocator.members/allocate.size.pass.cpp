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

// UNSUPPORTED: no-exceptions
// <memory>

// allocator:
// constexpr T* allocate(size_t n);

// ADDITIONAL_COMPILE_DEFINITIONS: _LIBCUDACXX_DISABLE_DEPRECATION_WARNINGS

#include <uscl/std/__memory_>
#include <uscl/std/cassert>

#include "test_macros.h"

#if TEST_HAS_EXCEPTIONS()
template <typename T>
void test_max(cuda::std::size_t count)
{
  cuda::std::allocator<T> a;
  try
  {
    TEST_IGNORE_NODISCARD a.allocate(count);
    assert(false);
  }
  catch (const std::bad_array_new_length&)
  {}
}

template <typename T>
void test()
{
  // Bug 26812 -- allocating too large
  typedef cuda::std::allocator<T> A;
  typedef cuda::std::allocator_traits<A> AT;
  A a;
  test_max<T>(AT::max_size(a) + 1); // just barely too large
  test_max<T>(AT::max_size(a) * 2); // significantly too large
  test_max<T>(((cuda::std::size_t) -1) / sizeof(T) + 1); // multiply will overflow
  test_max<T>((cuda::std::size_t) -1); // way too large
}
#endif // TEST_HAS_EXCEPTIONS()

int main(int, char**)
{
#if TEST_HAS_EXCEPTIONS()
  NV_IF_TARGET(NV_IS_HOST, (test<double>();));
  NV_IF_TARGET(NV_IS_HOST, (test<const double>();));
#endif // TEST_HAS_EXCEPTIONS()

  return 0;
}
