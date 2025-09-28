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

// Test that iota_view conforms to range and view concepts.

#include <uscl/std/ranges>

#include "types.h"

struct Decrementable
{
  using difference_type = int;

#if TEST_HAS_SPACESHIP()
  auto operator<=>(const Decrementable&) const = default;
#else
  __host__ __device__ bool operator==(const Decrementable&) const;
  __host__ __device__ bool operator!=(const Decrementable&) const;

  __host__ __device__ bool operator<(const Decrementable&) const;
  __host__ __device__ bool operator<=(const Decrementable&) const;
  __host__ __device__ bool operator>(const Decrementable&) const;
  __host__ __device__ bool operator>=(const Decrementable&) const;
#endif

  __host__ __device__ Decrementable& operator++();
  __host__ __device__ Decrementable operator++(int);
  __host__ __device__ Decrementable& operator--();
  __host__ __device__ Decrementable operator--(int);
};

struct Incrementable
{
  using difference_type = int;

#if TEST_HAS_SPACESHIP()
  auto operator<=>(const Incrementable&) const = default;
#else
  __host__ __device__ bool operator==(const Incrementable&) const;
  __host__ __device__ bool operator!=(const Incrementable&) const;

  __host__ __device__ bool operator<(const Incrementable&) const;
  __host__ __device__ bool operator<=(const Incrementable&) const;
  __host__ __device__ bool operator>(const Incrementable&) const;
  __host__ __device__ bool operator>=(const Incrementable&) const;
#endif

  __host__ __device__ Incrementable& operator++();
  __host__ __device__ Incrementable operator++(int);
};

static_assert(cuda::std::ranges::random_access_range<cuda::std::ranges::iota_view<int>>);
static_assert(cuda::std::ranges::random_access_range<const cuda::std::ranges::iota_view<int>>);
static_assert(cuda::std::ranges::bidirectional_range<cuda::std::ranges::iota_view<Decrementable>>);
static_assert(cuda::std::ranges::forward_range<cuda::std::ranges::iota_view<Incrementable>>);
static_assert(cuda::std::ranges::input_range<cuda::std::ranges::iota_view<NotIncrementable>>);
static_assert(cuda::std::ranges::view<cuda::std::ranges::iota_view<int>>);

int main(int, char**)
{
  return 0;
}
