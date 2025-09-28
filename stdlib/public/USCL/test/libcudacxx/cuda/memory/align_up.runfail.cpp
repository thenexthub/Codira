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

#include "test_macros.h"

__host__ __device__ bool test()
{
  uintptr_t ptr_int = 10;
  auto ptr          = reinterpret_cast<int*>(ptr_int);
  unused(cuda::align_up(ptr, 7)); // not power of two
  unused(cuda::align_up(ptr, 2)); // alignment smaller than alignof(int)
  unused(cuda::align_up(ptr, 4)); // wrong pointer alignment
  return true;
}

int main(int, char**)
{
  assert(test());
  return 0;
}
