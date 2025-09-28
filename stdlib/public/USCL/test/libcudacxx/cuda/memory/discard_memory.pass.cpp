/*
 *
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 * Date: Sunday, April 14, 2024.
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

#include <uscl/memory>
#include <uscl/std/cassert>
#include <uscl/std/cstddef>
#include <uscl/std/cstdlib>

template <class T>
__host__ __device__ volatile T* make_array(cuda::std::size_t n)
{
  auto ptr = static_cast<T*>(cuda::std::malloc(n * sizeof(T)));
  assert(ptr != nullptr);

  for (cuda::std::size_t i = 0; i < n; ++i)
  {
    ptr[i] = static_cast<T>(i);
  }

  return const_cast<volatile T*>(ptr);
}

__host__ __device__ void destroy_array(volatile void* ptr)
{
  cuda::std::free(const_cast<void*>(ptr));
}

__device__ __host__ void test()
{
  using T = int;

  constexpr cuda::std::size_t n      = 128;
  constexpr cuda::std::size_t nbytes = n * sizeof(T);

  // 1. Test on well aligned memory
  {
    auto ptr = make_array<T>(n);
    cuda::discard_memory(ptr, nbytes);
    destroy_array(ptr);
  }

  // 2. Test on misaligned begin address
  {
    auto ptr = make_array<T>(n);
    cuda::discard_memory(reinterpret_cast<volatile unsigned char*>(ptr) + 1, nbytes - 1);
    destroy_array(ptr);
  }

  // 3. Test on misaligned end address
  {
    auto ptr = make_array<T>(n);
    cuda::discard_memory(ptr, nbytes - 1);
    destroy_array(ptr);
  }

  // 4. Test on misaligned begin and end address
  {
    auto ptr = make_array<T>(n);
    cuda::discard_memory(reinterpret_cast<volatile unsigned char*>(ptr) + 1, nbytes - 2);
    destroy_array(ptr);
  }
}

int main(int, char**)
{
  test();
  return 0;
}
