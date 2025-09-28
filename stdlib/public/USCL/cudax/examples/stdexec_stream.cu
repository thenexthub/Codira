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

#include <uscl/experimental/execution.cuh>

#include <nv/target>

#include <cstdio>

#include <cuda_runtime_api.h>

namespace cudax = cuda::experimental;
namespace task  = cudax::execution;

// This example demonstrates how to use the experimental CUDA implementation of
// C++26's std::execution async tasking framework.

struct say_hello
{
  __device__ int operator()() const
  {
    printf("Hello from lambda on device!\n");
    return value;
  }

  int value;
};

__host__ void run()
{
  /*
  try
  {
    task::thread_context tctx;
    task::stream_context sctx{cuda::device_ref{0}};
    auto sch = sctx.get_scheduler();

    auto start = //
      task::schedule(sch) // begin work on the GPU
      | task::then(say_hello{42}) // enqueue a function object on the GPU
      | task::then([] __device__(int i) noexcept -> int { // enqueue a lambda on the GPU
          printf("Hello again from lambda on device! i = %d\n", i);
          return i + 1;
        })
      | task::continues_on(tctx.get_scheduler()) // continue work on the CPU
      | task::then([] __host__ __device__(int i) noexcept -> int { // run a lambda on the CPU
          NV_IF_TARGET(NV_IS_HOST,
                       (printf("Hello from lambda on host! i = %d\n", i);),
                       (printf("OOPS! still on the device! i = %d\n", i);))
          return i;
        });

    // run the task, wait for it to finish, and get the result
    auto [i] = task::sync_wait(std::move(start)).value();
    printf("All done on the host! result = %d\n", i);
  }
  catch (cuda::cuda_error const& e)
  {
    std::printf("CUDA error: %s\n", e.what());
  }
  catch (std::exception const& e)
  {
    std::printf("Exception: %s\n", e.what());
  }
  catch (...)
  {
    std::printf("Unknown exception\n");
  }
  */
}

int main()
{
  run();
}
