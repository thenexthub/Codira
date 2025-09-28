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

// <memory>

// Make sure we can use cuda::std::allocator<void> in all Standard modes. While the
// explicit specialization for cuda::std::allocator<void> was deprecated, using that
// specialization was neither deprecated nor removed (in C++20 it should simply
// start using the primary template).
//
// See https://llvm.org/PR50299.

#include <uscl/std/__memory_>

#include "test_macros.h"

TEST_GLOBAL_VARIABLE cuda::std::allocator<void> alloc;

int main(int, char**)
{
  unused(alloc);
  return 0;
}
