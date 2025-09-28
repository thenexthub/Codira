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

// transform_view::<iterator>::operator[]

#include <uscl/std/ranges>

#include "../types.h"
#include "test_macros.h"

__host__ __device__ constexpr bool test()
{
  int buff[8] = {0, 1, 2, 3, 4, 5, 6, 7};
  cuda::std::ranges::transform_view transformView1(MoveOnlyView{buff}, PlusOneMutable{});
  auto iter1 = cuda::std::move(transformView1).begin() + 1;
  assert(iter1[0] == 2);
  assert(iter1[4] == 6);

  static_assert(
    !noexcept(cuda::std::declval<
              cuda::std::ranges::iterator_t<cuda::std::ranges::transform_view<MoveOnlyView, PlusOneMutable>>>()[0]));
  static_assert(
    noexcept(cuda::std::declval<
             cuda::std::ranges::iterator_t<cuda::std::ranges::transform_view<MoveOnlyView, PlusOneNoexcept>>>()[0]));

  static_assert(
    cuda::std::is_same_v<
      int,
      decltype(cuda::std::declval<cuda::std::ranges::transform_view<RandomAccessView, PlusOneMutable>>().begin()[0])>);
  static_assert(
    cuda::std::is_same_v<
      int&,
      decltype(cuda::std::declval<cuda::std::ranges::transform_view<RandomAccessView, Increment>>().begin()[0])>);
  static_assert(cuda::std::is_same_v<
                int&&,
                decltype(cuda::std::declval<cuda::std::ranges::transform_view<RandomAccessView, IncrementRvalueRef>>()
                           .begin()[0])>);

  return true;
}

int main(int, char**)
{
  test();
#if defined(_CCCL_BUILTIN_ADDRESSOF)
  static_assert(test(), "");
#endif // _CCCL_BUILTIN_ADDRESSOF

  return 0;
}
