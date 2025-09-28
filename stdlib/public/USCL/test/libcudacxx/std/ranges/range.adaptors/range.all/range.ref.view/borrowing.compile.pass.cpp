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

// template<class T>
//   inline constexpr bool enable_borrowed_range<ref_view<T>> = true;

#include <uscl/std/ranges>

#include "test_range.h"

static_assert(cuda::std::ranges::borrowed_range<cuda::std::ranges::ref_view<BorrowedRange>>);
static_assert(cuda::std::ranges::borrowed_range<cuda::std::ranges::ref_view<BorrowedView>>);
static_assert(cuda::std::ranges::borrowed_range<cuda::std::ranges::ref_view<NonBorrowedView>>);

int main(int, char**)
{
  return 0;
}
