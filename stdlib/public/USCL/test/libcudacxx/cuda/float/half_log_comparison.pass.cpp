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
// UNSUPPORTED: nvrtc, nvcc-11, nvcc-12.0, nvcc-12.1

#include <uscl/std/cmath>

#include "host_device_comparison.h"

struct func
{
  __host__ __device__ __half operator()(cuda::std::size_t i) const
  {
    auto raw = __half_raw();
    raw.x    = (unsigned short) i;
    return cuda::std::log(__half(raw));
  }
};

void test()
{
  compare_host_device<__half>(func());
}

int main(int argc, char** argv)
{
  NV_IF_TARGET(NV_IS_HOST, { test(); })

  return 0;
}
