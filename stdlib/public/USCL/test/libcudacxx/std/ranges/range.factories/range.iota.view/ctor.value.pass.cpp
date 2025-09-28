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

// constexpr explicit iota_view(W value);

#include <uscl/std/cassert>
#include <uscl/std/ranges>

#include "test_macros.h"
#include "types.h"

struct SomeIntComparable
{
  using difference_type = int;

  SomeInt value_;
  __host__ __device__ constexpr SomeIntComparable()
      : value_(SomeInt(10))
  {}

  __host__ __device__ friend constexpr bool operator==(SomeIntComparable lhs, SomeIntComparable rhs)
  {
    return lhs.value_ == rhs.value_;
  }
  __host__ __device__ friend constexpr bool operator==(SomeIntComparable lhs, SomeInt rhs)
  {
    return lhs.value_ == rhs;
  }
  __host__ __device__ friend constexpr bool operator==(SomeInt lhs, SomeIntComparable rhs)
  {
    return lhs == rhs.value_;
  }
#if TEST_STD_VER < 2020
  __host__ __device__ friend constexpr bool operator!=(SomeIntComparable lhs, SomeIntComparable rhs)
  {
    return lhs.value_ != rhs.value_;
  }
  __host__ __device__ friend constexpr bool operator!=(SomeIntComparable lhs, SomeInt rhs)
  {
    return lhs.value_ != rhs;
  }
  __host__ __device__ friend constexpr bool operator!=(SomeInt lhs, SomeIntComparable rhs)
  {
    return lhs != rhs.value_;
  }
#endif // TEST_STD_VER < 2020

  __host__ __device__ friend constexpr difference_type operator-(SomeIntComparable lhs, SomeIntComparable rhs)
  {
    return lhs.value_ - rhs.value_;
  }

  __host__ __device__ constexpr SomeIntComparable& operator++()
  {
    ++value_;
    return *this;
  }
  __host__ __device__ constexpr SomeIntComparable operator++(int)
  {
    auto tmp = *this;
    ++value_;
    return tmp;
  }
  __host__ __device__ constexpr SomeIntComparable operator--()
  {
    --value_;
    return *this;
  }
};

__host__ __device__ constexpr bool test()
{
  {
    cuda::std::ranges::iota_view<SomeInt> io(SomeInt(42));
    assert((*io.begin()).value_ == 42);
    // Check that end returns cuda::std::unreachable_sentinel.
    assert(io.end() != io.begin());
    static_assert(cuda::std::same_as<decltype(io.end()), cuda::std::unreachable_sentinel_t>);
  }

  {
    cuda::std::ranges::iota_view<SomeInt, SomeIntComparable> io(SomeInt(0));
    assert(cuda::std::ranges::next(io.begin(), 10) == io.end());
  }
  {
    static_assert(!cuda::std::is_convertible_v<cuda::std::ranges::iota_view<SomeInt>, SomeInt>);
    static_assert(cuda::std::is_constructible_v<cuda::std::ranges::iota_view<SomeInt>, SomeInt>);
  }

  return true;
}

int main(int, char**)
{
  test();
  static_assert(test(), "");

  return 0;
}
