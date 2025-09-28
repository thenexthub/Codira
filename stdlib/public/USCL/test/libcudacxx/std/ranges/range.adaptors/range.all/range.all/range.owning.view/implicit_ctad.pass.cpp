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

// owning_view

// Make sure that the implicitly-generated CTAD works.

#include <uscl/std/ranges>
#include <uscl/std/utility>

#include "test_macros.h"

struct Range
{
  __host__ __device__ int* begin();
  __host__ __device__ int* end();
};

int main(int, char**)
{
  Range r;
  cuda::std::ranges::owning_view view{cuda::std::move(r)};
  unused(view);
  static_assert(cuda::std::is_same_v<decltype(view), cuda::std::ranges::owning_view<Range>>);

  return 0;
}
