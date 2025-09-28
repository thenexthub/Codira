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

// UNSUPPORTED: sanitizer-new-delete

// It is not possible to overwrite device operator new
// UNSUPPORTED: true

// template<class T>
//   constexpr unique_ptr<T> make_unique_for_overwrite(); // T is not array
//
// template<class T>
//   constexpr unique_ptr<T> make_unique_for_overwrite(size_t n); // T is U[]

// Test the object is not value initialized

#include <uscl/std/__memory_>
#include <uscl/std/cassert>
#include <uscl/std/concepts>
#include <uscl/std/cstddef>
#include <uscl/std/cstdlib>

#include "test_macros.h"

TEST_DIAG_SUPPRESS_MSVC(4310) // cast truncates constant value

constexpr char pattern = (char) 0xDE;

void* operator new(cuda::std::size_t count)
{
  void* ptr = malloc(count);
  for (cuda::std::size_t i = 0; i < count; ++i)
  {
    *(reinterpret_cast<char*>(ptr) + i) = pattern;
  }
  return ptr;
}

void* operator new[](cuda::std::size_t count)
{
  return ::operator new(count);
}

void operator delete(void* ptr) noexcept
{
  free(ptr);
}

void operator delete[](void* ptr) noexcept
{
  ::operator delete(ptr);
}

#if TEST_COMPILER(GCC)
void operator delete(void* ptr, cuda::std::size_t) noexcept
{
  free(ptr);
}
void operator delete[](void* ptr, cuda::std::size_t) noexcept
{
  ::operator delete(ptr);
}
#endif // TEST_COMPILER(GCC)

__host__ __device__ void test()
{
  {
    decltype(auto) ptr = cuda::std::make_unique_for_overwrite<int>();
    static_assert(cuda::std::same_as<cuda::std::unique_ptr<int>, decltype(ptr)>, "");
    NV_IF_TARGET(NV_IS_HOST, (assert(*(reinterpret_cast<char*>(ptr.get())) == pattern);))
  }
  {
    decltype(auto) ptr = cuda::std::make_unique_for_overwrite<int[]>(3);
    static_assert(cuda::std::same_as<cuda::std::unique_ptr<int[]>, decltype(ptr)>, "");
    NV_IF_TARGET(NV_IS_HOST, (assert(*(reinterpret_cast<char*>(&ptr[0])) == pattern);))
    NV_IF_TARGET(NV_IS_HOST, (assert(*(reinterpret_cast<char*>(&ptr[1])) == pattern);))
    NV_IF_TARGET(NV_IS_HOST, (assert(*(reinterpret_cast<char*>(&ptr[2])) == pattern);))
  }
}

int main(int, char**)
{
  test();

  return 0;
}
