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

// template <class T>
// views::repeat(T &&) requires constructible_from<ranges::repeat_view<T>, T>;

// template <class T, class Bound>
// views::repeat(T &&, Bound &&) requires constructible_from<ranges::repeat_view<T, Bound>, T, Bound>;

#include <uscl/std/cassert>
#include <uscl/std/concepts>
#include <uscl/std/ranges>
#include <uscl/std/tuple>
#include <uscl/std/type_traits>

#include "MoveOnly.h"
#include "test_macros.h"

struct NonCopyable
{
  __host__ __device__ NonCopyable(NonCopyable&) = delete;
};

struct NonDefaultCtor
{
  __host__ __device__ NonDefaultCtor(int) {}
};

struct Empty
{};

struct LessThan3
{
  __host__ __device__ constexpr bool operator()(int i) const
  {
    return i < 3;
  }
};

struct EqualTo33
{
  __host__ __device__ constexpr bool operator()(int i) const
  {
    return i == 33;
  }
};

struct Add3
{
  __host__ __device__ constexpr int operator()(int i) const
  {
    return i + 3;
  }
};

// Tp is_object
static_assert(cuda::std::is_invocable_v<decltype(cuda::std::views::repeat), int>);
static_assert(!cuda::std::is_invocable_v<decltype(cuda::std::views::repeat), void>);

// _Bound is semiregular, integer like or cuda::std::unreachable_sentinel_t
static_assert(!cuda::std::is_invocable_v<decltype(cuda::std::views::repeat), int, Empty>);
static_assert(!cuda::std::is_invocable_v<decltype(cuda::std::views::repeat), int, NonCopyable>);
static_assert(!cuda::std::is_invocable_v<decltype(cuda::std::views::repeat), int, NonDefaultCtor>);
static_assert(cuda::std::is_invocable_v<decltype(cuda::std::views::repeat), int, cuda::std::unreachable_sentinel_t>);

// Tp is copy_constructible
static_assert(!cuda::std::is_invocable_v<decltype(cuda::std::views::repeat), NonCopyable>);

// Tp is move_constructible
static_assert(cuda::std::is_invocable_v<decltype(cuda::std::views::repeat), MoveOnly>);

__host__ __device__ constexpr bool test()
{
  assert(*cuda::std::views::repeat(33).begin() == 33);
  assert(*cuda::std::views::repeat(33, 10).begin() == 33);
  static_assert(cuda::std::same_as<decltype(cuda::std::views::repeat(42)), cuda::std::ranges::repeat_view<int>>);
  static_assert(
    cuda::std::same_as<decltype(cuda::std::views::repeat(42, 3)), cuda::std::ranges::repeat_view<int, int>>);
  static_assert(cuda::std::same_as<decltype(cuda::std::views::repeat), decltype(cuda::std::ranges::views::repeat)>);

#if 0 // Not yet implemented views
  // unbound && drop_view
  {
    auto r = cuda::std::views::repeat(33) | cuda::std::views::drop(3);
    static_assert(!cuda::std::ranges::sized_range<decltype(r)>);
    assert(*r.begin() == 33);
  }

  // bound && drop_view
  {
    auto r = cuda::std::views::repeat(33, 8) | cuda::std::views::drop(3);
    static_assert(cuda::std::ranges::sized_range<decltype(r)>);
    assert(*r.begin() == 33);
    assert(r.size() == 5);
  }
#endif // Not yet implemented views

  // unbound && take_view
  {
    auto r = cuda::std::views::repeat(33) | cuda::std::views::take(3);
    static_assert(cuda::std::ranges::sized_range<decltype(r)>);
    assert(*r.begin() == 33);
    assert(r.size() == 3);
  }

  // bound && take_view
  {
    auto r = cuda::std::views::repeat(33, 8) | cuda::std::views::take(3);
    static_assert(cuda::std::ranges::sized_range<decltype(r)>);
    assert(*r.begin() == 33);
    assert(r.size() == 3);
  }

  // bound && transform_view
  {
    auto r = cuda::std::views::repeat(33, 8) | cuda::std::views::transform(Add3{});
    assert(*r.begin() == 36);
    assert(r.size() == 8);
  }

  // unbound && transform_view
  {
    auto r = cuda::std::views::repeat(33) | cuda::std::views::transform(Add3{});
    assert(*r.begin() == 36);
  }

  return true;
}

int main(int, char**)
{
  test();
  static_assert(test());

  return 0;
}
