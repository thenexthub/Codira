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

#if defined(__clang__)
#  pragma clang diagnostic ignored "-Wsign-compare"
#elif defined(__GNUC__)
#  pragma GCC diagnostic ignored "-Wsign-compare"
#elif defined(_MSC_VER)
#  pragma warning(disable : 4018 4389) // various "signed/unsigned mismatch"
#endif

// constexpr auto end() const;
// constexpr iterator end() const requires same_as<W, Bound>;

#include <uscl/std/cassert>
#include <uscl/std/ranges>
#include <uscl/std/utility>

#include "test_macros.h"
#include "types.h"

template <class T, class U>
__host__ __device__ constexpr void testType(U u)
{
  {
    cuda::std::ranges::iota_view<T, U> io(T(0), u);
    assert(cuda::std::ranges::next(io.begin(), 10) == io.end());
  }
  {
    cuda::std::ranges::iota_view<T, U> io(T(10), u);
    assert(io.begin() == io.end());
    assert(io.begin() == cuda::std::move(io).end());
  }
  {
    const cuda::std::ranges::iota_view<T, U> io(T(0), u);
    assert(cuda::std::ranges::next(io.begin(), 10) == io.end());
    assert(cuda::std::ranges::next(io.begin(), 10) == cuda::std::move(io).end());
  }
  {
    const cuda::std::ranges::iota_view<T, U> io(T(10), u);
    assert(io.begin() == io.end());
  }

  {
    cuda::std::ranges::iota_view<T> io(T(0), cuda::std::unreachable_sentinel);
    assert(io.begin() != io.end());
    assert(cuda::std::ranges::next(io.begin()) != io.end());
    assert(cuda::std::ranges::next(io.begin(), 10) != io.end());
  }
  {
    const cuda::std::ranges::iota_view<T> io(T(0), cuda::std::unreachable_sentinel);
    assert(io.begin() != io.end());
    assert(cuda::std::ranges::next(io.begin()) != io.end());
    assert(cuda::std::ranges::next(io.begin(), 10) != io.end());
  }
}

__host__ __device__ constexpr bool test()
{
  testType<SomeInt>(SomeInt(10));
  testType<SomeInt>(IntComparableWith(SomeInt(10)));
  testType<signed long>(IntComparableWith<signed long>(10));
  testType<unsigned long>(IntComparableWith<unsigned long>(10));
  testType<int>(IntComparableWith<int>(10));
  testType<int>(int(10));
  testType<int>(unsigned(10));
  testType<unsigned>(unsigned(10));
  testType<unsigned>(int(10));
  testType<unsigned>(IntComparableWith<unsigned>(10));
  testType<short>(short(10));
  testType<short>(IntComparableWith<short>(10));
  testType<unsigned short>(IntComparableWith<unsigned short>(10));

  return true;
}

int main(int, char**)
{
  test();
  static_assert(test(), "");

  return 0;
}
