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

// We compile with CCCL_ENABLE_ASSERTIONS, but want to disable assertions
#undef CCCL_ENABLE_ASSERTIONS

#include <uscl/std/cassert>

__host__ __device__ inline bool failed_on_device()
{
  NV_IF_ELSE_TARGET(NV_IS_DEVICE, return false;, return true;)
}

int main(int, char**)
{
  _CCCL_ASSERT(failed_on_device(), "Should fail on device");
  return 0;
}
