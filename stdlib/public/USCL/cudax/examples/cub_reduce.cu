/*
 *
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 * Date: Wednesday, July 24, 2024.
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

// Example of using `cub::DeviceReduce::Reduce` with cudax environment.

#include <cub/device/device_reduce.cuh>

#include <uscl/experimental/container.cuh>
#include <uscl/experimental/memory_resource.cuh>
#include <uscl/experimental/stream.cuh>

#include <iostream>

namespace cudax = cuda::experimental;

int main()
{
  constexpr int num_items = 50000;

  // A CUDA stream on which to execute the reduction
  cudax::stream stream{cuda::devices[0]};
  cudax::device_memory_resource mr{cuda::devices[0]};

  // Allocate input and output, but do not zero initialize output (`cudax::no_init`)
  auto d_in  = cudax::make_async_buffer<int>(stream, mr, num_items, 1);
  auto d_out = cudax::make_async_buffer<float>(stream, mr, 1, cudax::no_init);

  // An environment we use to pass all necessary information to CUB
  cudax::env_t<cuda::mr::device_accessible> env{mr, stream};
  cub::DeviceReduce::Reduce(d_in.begin(), d_out.begin(), num_items, cuda::std::plus{}, 0, env);

  auto h_out = cudax::make_async_buffer<float>(stream, cudax::pinned_memory_resource{}, d_out);

  stream.sync();

  if (h_out.get_unsynchronized(0) != num_items)
  {
    std::cerr << "Result verification failed: " << h_out.get_unsynchronized(0) << " != " << num_items << "\n";
    exit(EXIT_FAILURE);
  }
}
