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

#include <uscl/__runtime/ensure_current_context.h>

#include <testing.cuh>

namespace driver = cuda::__driver;

void recursive_check_device_setter(int id)
{
  int cudart_id;
  cuda::__ensure_current_context setter(cuda::device_ref{id});
  CCCLRT_REQUIRE(test::count_driver_stack() == cuda::devices.size() - id);
  auto ctx = driver::__ctxGetCurrent();
  CUDART(cudaGetDevice(&cudart_id));
  CCCLRT_REQUIRE(cudart_id == id);

  if (id != 0)
  {
    recursive_check_device_setter(id - 1);

    CCCLRT_REQUIRE(test::count_driver_stack() == cuda::devices.size() - id);
    CCCLRT_REQUIRE(ctx == driver::__ctxGetCurrent());
    CUDART(cudaGetDevice(&cudart_id));
    CCCLRT_REQUIRE(cudart_id == id);
  }
}

C2H_TEST("ensure current context", "[device]")
{
  test::empty_driver_stack();
  // If possible use something different than CUDART default 0
  int target_device = static_cast<int>(cuda::devices.size() - 1);

  SECTION("context setter")
  {
    recursive_check_device_setter(target_device);

    CCCLRT_REQUIRE(test::count_driver_stack() == 0);
  }
}
