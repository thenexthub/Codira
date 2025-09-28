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
#include <uscl/std/type_traits>

template <typename T, typename U, typename V>
__host__ __device__ void test()
{
  uintptr_t ptr_int         = 16;
  [[maybe_unused]] auto ptr = reinterpret_cast<T>(ptr_int);
  static_assert(cuda::std::is_same_v<U, decltype(cuda::ptr_rebind<V>(ptr))>);
}

__host__ __device__ bool test()
{
  test<char*, char*, char>();
  test<char*, short*, short>();
  test<char*, int*, int>();
  test<char*, void*, void>();
  test<const char*, const int*, int>();
  test<volatile char*, volatile int*, int>();
  test<const volatile char*, const volatile int*, int>();
  test<const char*, const void*, void>();
  return true;
}

__global__ void test_kernel()
{
  __shared__ int smem_value[4];
  auto ptr = smem_value;
  assert(cuda::device::is_address_from(cuda::ptr_rebind<uint64_t>(ptr), cuda::device::address_space::shared));
}

int main(int, char**)
{
  assert(test());
  NV_IF_TARGET(NV_IS_HOST, (test_kernel<<<1, 1>>>(); assert(cudaDeviceSynchronize() == cudaSuccess);))
  return 0;
}
