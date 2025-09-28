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
#include <uscl/std/cstdint>

template <typename T, typename U>
__host__ __device__ void test()
{
  uintptr_t ptr_int = 10;
  auto ptr          = reinterpret_cast<T>(ptr_int);
  assert(cuda::align_down(ptr, 1) == ptr);
  assert(cuda::align_down(ptr, 2) == ptr);
  assert(cuda::align_down(ptr, 4) == reinterpret_cast<T>(8));
  assert(cuda::align_down(ptr, 8) == reinterpret_cast<T>(8));
  uintptr_t ptr_int2 = 12;
  auto ptr2          = reinterpret_cast<U>(ptr_int2);
  assert(cuda::align_down(ptr2, 8) == reinterpret_cast<U>(8));
  size_t align = 8;
  assert(cuda::align_down(ptr2, align) == reinterpret_cast<U>(8)); // run-time alignment
}

__host__ __device__ bool test()
{
  test<char*, int*>();
  test<const char*, const int*>();
  test<volatile char*, volatile int*>();
  test<const volatile char*, const volatile int*>();
  test<void*, void*>();
  return true;
}

__global__ void test_kernel()
{
  __shared__ int smem_value[4];
  auto ptr = smem_value;
  assert(cuda::device::is_address_from(ptr + 3, cuda::device::address_space::shared));
  assert(cuda::device::is_address_from(cuda::align_down(ptr + 3, 8), cuda::device::address_space::shared));
}

int main(int, char**)
{
  assert(test());
  NV_IF_TARGET(NV_IS_HOST, (test_kernel<<<1, 1>>>(); assert(cudaDeviceSynchronize() == cudaSuccess);))
  return 0;
}
