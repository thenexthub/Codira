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

// template<viewable_range R>
// using all_t = decltype(views::all(declval<R>()));

#include <uscl/std/ranges>

#include "test_iterators.h"

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

struct BorrowableRange
{
  __host__ __device__ int* begin() const;
  __host__ __device__ int* end() const;
};
template <>
inline constexpr bool cuda::std::ranges::enable_borrowed_range<BorrowableRange> = true;

template <class T, class = void>
constexpr bool HasAllT = false;

template <class T>
constexpr bool HasAllT<T, cuda::std::void_t<cuda::std::views::all_t<T>>> = true;

// When T is a view, returns decay-copy(T)
static_assert(cuda::std::is_same_v<cuda::std::views::all_t<View>, View>);
static_assert(cuda::std::is_same_v<cuda::std::views::all_t<View&>, View>);
static_assert(cuda::std::is_same_v<cuda::std::views::all_t<View&&>, View>);
static_assert(cuda::std::is_same_v<cuda::std::views::all_t<const View>, View>);
static_assert(cuda::std::is_same_v<cuda::std::views::all_t<const View&>, View>);
static_assert(cuda::std::is_same_v<cuda::std::views::all_t<const View&&>, View>);

// Otherwise, when T is a reference to a range, returns ref_view<T>
static_assert(cuda::std::is_same_v<cuda::std::views::all_t<Range&>, cuda::std::ranges::ref_view<Range>>);
static_assert(cuda::std::is_same_v<cuda::std::views::all_t<const Range&>, cuda::std::ranges::ref_view<const Range>>);
static_assert(
  cuda::std::is_same_v<cuda::std::views::all_t<BorrowableRange&>, cuda::std::ranges::ref_view<BorrowableRange>>);
static_assert(cuda::std::is_same_v<cuda::std::views::all_t<const BorrowableRange&>,
                                   cuda::std::ranges::ref_view<const BorrowableRange>>);

// Otherwise, returns owning_view<T>
static_assert(cuda::std::is_same_v<cuda::std::views::all_t<Range>, cuda::std::ranges::owning_view<Range>>);
static_assert(cuda::std::is_same_v<cuda::std::views::all_t<Range&&>, cuda::std::ranges::owning_view<Range>>);
static_assert(!HasAllT<const Range>, "");
static_assert(!HasAllT<const Range&&>, "");
static_assert(
  cuda::std::is_same_v<cuda::std::views::all_t<BorrowableRange>, cuda::std::ranges::owning_view<BorrowableRange>>);
static_assert(
  cuda::std::is_same_v<cuda::std::views::all_t<BorrowableRange&&>, cuda::std::ranges::owning_view<BorrowableRange>>);
static_assert(!HasAllT<const BorrowableRange>, "");
static_assert(!HasAllT<const BorrowableRange&&>, "");

int main(int, char**)
{
  return 0;
}
