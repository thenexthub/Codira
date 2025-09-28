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

// The requirements for transform_view::<iterator>'s members.

#include <uscl/std/ranges>

#include "../types.h"
#include "test_macros.h"

static_assert(cuda::std::ranges::bidirectional_range<cuda::std::ranges::transform_view<BidirectionalView, PlusOne>>);
static_assert(!cuda::std::ranges::bidirectional_range<cuda::std::ranges::transform_view<ForwardView, PlusOne>>);

static_assert(cuda::std::ranges::random_access_range<cuda::std::ranges::transform_view<RandomAccessView, PlusOne>>);
static_assert(!cuda::std::ranges::random_access_range<cuda::std::ranges::transform_view<BidirectionalView, PlusOne>>);

int main(int, char**)
{
  return 0;
}
