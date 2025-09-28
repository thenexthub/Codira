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

// template<class R, class F>
//   transform_view(R&&, F) -> transform_view<views::all_t<R>, F>;

#include <uscl/std/cassert>
#include <uscl/std/concepts>
#include <uscl/std/ranges>
#include <uscl/std/utility>

#include "test_macros.h"

struct PlusOne
{
  __host__ __device__ int operator()(int x) const;
};

struct View : cuda::std::ranges::view_base
{
  __host__ __device__ int* begin() const;
  __host__ __device__ int* end() const;
};

struct Range
{
  __host__ __device__ int* begin() const;
  __host__ __device__ int* end() const;
};

struct BorrowedRange
{
  __host__ __device__ int* begin() const;
  __host__ __device__ int* end() const;
};
template <>
inline constexpr bool cuda::std::ranges::enable_borrowed_range<BorrowedRange> = true;

// gcc falls over it feet trying to evaluate this otherwise
using result_rvlaue_range = cuda::std::ranges::transform_view<cuda::std::ranges::owning_view<Range>, PlusOne>;
using result_rvlaue_borrowed_range =
  cuda::std::ranges::transform_view<cuda::std::ranges::owning_view<BorrowedRange>, PlusOne>;

__host__ __device__ void testCTAD()
{
  View v;
  Range r;
  BorrowedRange br;
  PlusOne f;

  static_assert(cuda::std::same_as<decltype(cuda::std::ranges::transform_view(v, f)),
                                   cuda::std::ranges::transform_view<View, PlusOne>>);
  static_assert(cuda::std::same_as<decltype(cuda::std::ranges::transform_view(cuda::std::move(v), f)),
                                   cuda::std::ranges::transform_view<View, PlusOne>>);
  static_assert(cuda::std::same_as<decltype(cuda::std::ranges::transform_view(r, f)),
                                   cuda::std::ranges::transform_view<cuda::std::ranges::ref_view<Range>, PlusOne>>);
  static_assert(
    cuda::std::same_as<decltype(cuda::std::ranges::transform_view(cuda::std::move(r), f)), result_rvlaue_range>);
  static_assert(
    cuda::std::same_as<decltype(cuda::std::ranges::transform_view(br, f)),
                       cuda::std::ranges::transform_view<cuda::std::ranges::ref_view<BorrowedRange>, PlusOne>>);
  static_assert(cuda::std::same_as<decltype(cuda::std::ranges::transform_view(cuda::std::move(br), f)),
                                   result_rvlaue_borrowed_range>);

  unused(v);
  unused(r);
  unused(br);
  unused(f);
}

int main(int, char**)
{
  return 0;
}
