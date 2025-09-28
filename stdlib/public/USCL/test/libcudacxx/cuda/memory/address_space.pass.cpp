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

struct MyStruct
{
  int v;
};

__device__ int global_var;
__constant__ int constant_var;

__global__ void test_kernel(const _CCCL_GRID_CONSTANT MyStruct grid_constant_var)
{
  using cuda::device::address_space;
  using cuda::device::is_address_from;
  using cuda::device::is_object_from;
  __shared__ int shared_var;
  int local_var;

  // 1. Test non-volatile pointers/objects
  {
    assert(is_address_from(&global_var, address_space::global));
    assert(is_address_from(&shared_var, address_space::shared));
    assert(is_address_from(&constant_var, address_space::constant));
    assert(is_address_from(&local_var, address_space::local));
    assert(is_address_from(&grid_constant_var, address_space::grid_constant) == _CCCL_HAS_GRID_CONSTANT());
    // todo: test address_space::cluster_shared

    assert(is_object_from(global_var, address_space::global));
    assert(is_object_from(shared_var, address_space::shared));
    assert(is_object_from(constant_var, address_space::constant));
    assert(is_object_from(local_var, address_space::local));
    assert(is_object_from(grid_constant_var, address_space::grid_constant) == _CCCL_HAS_GRID_CONSTANT());
    // todo: test address_space::cluster_shared
  }

  // 2. Test volatile pointers/objects
  {
    volatile auto& v_global_var        = global_var;
    volatile auto& v_shared_var        = shared_var;
    volatile auto& v_constant_var      = constant_var;
    volatile auto& v_local_var         = local_var;
    volatile auto& v_grid_constant_var = grid_constant_var;

    assert(is_address_from(&v_global_var, address_space::global));
    assert(is_address_from(&v_shared_var, address_space::shared));
    assert(is_address_from(&v_constant_var, address_space::constant));
    assert(is_address_from(&v_local_var, address_space::local));
    assert(is_address_from(&v_grid_constant_var, address_space::grid_constant) == _CCCL_HAS_GRID_CONSTANT());
    // todo: test address_space::cluster_shared

    assert(is_object_from(v_global_var, address_space::global));
    assert(is_object_from(v_shared_var, address_space::shared));
    assert(is_object_from(v_constant_var, address_space::constant));
    assert(is_object_from(v_local_var, address_space::local));
    assert(is_object_from(v_grid_constant_var, address_space::grid_constant) == _CCCL_HAS_GRID_CONSTANT());
    // todo: test address_space::cluster_shared
  }
}

int main(int, char**)
{
  NV_IF_TARGET(NV_IS_HOST, (test_kernel<<<1, 1>>>(MyStruct{}); assert(cudaDeviceSynchronize() == cudaSuccess);))
  return 0;
}
