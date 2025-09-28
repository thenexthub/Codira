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

// views::iota

#include <uscl/std/cassert>
#include <uscl/std/concepts>
#include <uscl/std/ranges>

#include "test_macros.h"
#include "types.h"

template <class T, class U>
__host__ __device__ constexpr void testType(U u)
{
  // Test that this generally does the right thing.
  // Test with only one argument.
  {
    assert(*cuda::std::views::iota(T(0)).begin() == T(0));
  }
  {
    const auto io = cuda::std::views::iota(T(10));
    assert(*io.begin() == T(10));
  }
  // Test with two arguments.
  {
    assert(*cuda::std::views::iota(T(0), u).begin() == T(0));
  }
  {
    const auto io = cuda::std::views::iota(T(10), u);
    assert(*io.begin() == T(10));
  }
  // Test that we return the correct type.
  {
    static_assert(cuda::std::is_same_v<decltype(cuda::std::views::iota(T(10))), cuda::std::ranges::iota_view<T>>);
    static_assert(cuda::std::is_same_v<decltype(cuda::std::views::iota(T(10), u)), cuda::std::ranges::iota_view<T, U>>);
  }
}

struct X
{};

__host__ __device__ constexpr bool test()
{
  testType<SomeInt>(SomeInt(10));
  testType<SomeInt>(IntComparableWith(SomeInt(10)));
  testType<signed long>(IntComparableWith<signed long>(10));
  testType<unsigned long>(IntComparableWith<unsigned long>(10));
  testType<int>(IntComparableWith<int>(10));
  testType<int>(int(10));
  testType<unsigned>(unsigned(10));
  testType<unsigned>(IntComparableWith<unsigned>(10));
  testType<short>(short(10));
  testType<short>(IntComparableWith<short>(10));
  testType<unsigned short>(IntComparableWith<unsigned short>(10));

  {
    static_assert(cuda::std::is_invocable_v<decltype(cuda::std::views::iota), int>);
    static_assert(!cuda::std::is_invocable_v<decltype(cuda::std::views::iota), X>);
    static_assert(cuda::std::is_invocable_v<decltype(cuda::std::views::iota), int, int>);
    static_assert(!cuda::std::is_invocable_v<decltype(cuda::std::views::iota), int, X>);
  }
  {
    static_assert(cuda::std::same_as<decltype(cuda::std::views::iota), decltype(cuda::std::ranges::views::iota)>);
  }

  return true;
}

int main(int, char**)
{
  test();
  static_assert(test(), "");

  return 0;
}
