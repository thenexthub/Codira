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

#include <uscl/__execution/determinism.h>

__host__ __device__ void test()
{
  namespace exec = cuda::execution;
  static_assert(cuda::std::is_base_of_v<exec::__requirement, exec::determinism::run_to_run_t>);
  static_assert(cuda::std::is_base_of_v<exec::__requirement, exec::determinism::not_guaranteed_t>);
  static_assert(cuda::std::is_base_of_v<exec::__requirement, exec::determinism::gpu_to_gpu_t>);

  static_assert(cuda::std::is_same_v<decltype(exec::determinism::__get_determinism(exec::determinism::run_to_run)),
                                     exec::determinism::run_to_run_t>);
  static_assert(cuda::std::is_same_v<decltype(exec::determinism::__get_determinism(exec::determinism::not_guaranteed)),
                                     exec::determinism::not_guaranteed_t>);
  static_assert(cuda::std::is_same_v<decltype(exec::determinism::__get_determinism(exec::determinism::gpu_to_gpu)),
                                     exec::determinism::gpu_to_gpu_t>);
}

int main(int, char**)
{
  test();

  return 0;
}
