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

// template<class T, class Bound>
//    repeat_view(T, Bound) -> repeat_view<T, Bound>;

#include <uscl/std/concepts>
#include <uscl/std/ranges>
#include <uscl/std/utility>

#include "test_macros.h"

struct Empty
{};

int main(int, char**)
{
  Empty empty{};

  // clang-format off
  static_assert(cuda::std::same_as<decltype(cuda::std::ranges::repeat_view(Empty{})), cuda::std::ranges::repeat_view<Empty>>);
#if 0 // No passing in any compiler, maybe C++23 only fix?
  static_assert(cuda::std::same_as<decltype(cuda::std::ranges::repeat_view(empty)), cuda::std::ranges::repeat_view<Empty>>);
#endif //
  static_assert(cuda::std::same_as<decltype(cuda::std::ranges::repeat_view(cuda::std::move(empty))), cuda::std::ranges::repeat_view<Empty>>);
  static_assert(cuda::std::same_as<decltype(cuda::std::ranges::repeat_view(10, 1)), cuda::std::ranges::repeat_view<int, int>>);
  static_assert(cuda::std::same_as<decltype(cuda::std::ranges::repeat_view(10, 1U)), cuda::std::ranges::repeat_view<int, unsigned>>);
  static_assert(cuda::std::same_as<decltype(cuda::std::ranges::repeat_view(10, 1UL)), cuda::std::ranges::repeat_view<int, unsigned long>>);
  // clang-format on

  unused(empty);

  return 0;
}
