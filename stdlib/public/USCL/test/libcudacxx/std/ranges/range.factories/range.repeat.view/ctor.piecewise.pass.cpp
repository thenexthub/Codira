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

// template<class... TArgs, class... BoundArgs>
//       requires constructible_from<T, TArgs...> &&
//                constructible_from<Bound, BoundArgs...>
//     constexpr explicit repeat_view(piecewise_construct_t,
//       tuple<TArgs...> value_args, tuple<BoundArgs...> bound_args = tuple<>{});

#include <uscl/std/cassert>
#include <uscl/std/concepts>
#include <uscl/std/ranges>
#include <uscl/std/tuple>
#include <uscl/std/utility>

struct C
{};

struct B
{
  int v;
};

struct A
{
  int x = 111;
  int y = 222;

  constexpr A() = default;
  __host__ __device__ constexpr A(B b)
      : x(b.v)
      , y(b.v + 1)
  {}
  __host__ __device__ constexpr A(int _x, int _y)
      : x(_x)
      , y(_y)
  {}
};

static_assert(cuda::std::constructible_from<cuda::std::ranges::repeat_view<A, int>,
                                            cuda::std::piecewise_construct_t,
                                            cuda::std::tuple<int, int>,
                                            cuda::std::tuple<int>>);
static_assert(cuda::std::constructible_from<cuda::std::ranges::repeat_view<A, int>,
                                            cuda::std::piecewise_construct_t,
                                            cuda::std::tuple<B>,
                                            cuda::std::tuple<int>>);
static_assert(cuda::std::constructible_from<cuda::std::ranges::repeat_view<A, int>,
                                            cuda::std::piecewise_construct_t,
                                            cuda::std::tuple<>,
                                            cuda::std::tuple<int>>);
static_assert(cuda::std::constructible_from<cuda::std::ranges::repeat_view<A>,
                                            cuda::std::piecewise_construct_t,
                                            cuda::std::tuple<int, int>,
                                            cuda::std::tuple<cuda::std::unreachable_sentinel_t>>);
static_assert(cuda::std::constructible_from<cuda::std::ranges::repeat_view<A>,
                                            cuda::std::piecewise_construct_t,
                                            cuda::std::tuple<B>,
                                            cuda::std::tuple<cuda::std::unreachable_sentinel_t>>);
static_assert(cuda::std::constructible_from<cuda::std::ranges::repeat_view<A>,
                                            cuda::std::piecewise_construct_t,
                                            cuda::std::tuple<>,
                                            cuda::std::tuple<cuda::std::unreachable_sentinel_t>>);
static_assert(!cuda::std::constructible_from<cuda::std::ranges::repeat_view<A, int>,
                                             cuda::std::piecewise_construct_t,
                                             cuda::std::tuple<C>,
                                             cuda::std::tuple<int>>);
static_assert(!cuda::std::constructible_from<cuda::std::ranges::repeat_view<A>,
                                             cuda::std::piecewise_construct_t,
                                             cuda::std::tuple<C>,
                                             cuda::std::tuple<cuda::std::unreachable_sentinel_t>>);
static_assert(!cuda::std::constructible_from<cuda::std::ranges::repeat_view<A, int>,
                                             cuda::std::piecewise_construct_t,
                                             cuda::std::tuple<int, int, int>,
                                             cuda::std::tuple<int>>);
static_assert(!cuda::std::constructible_from<cuda::std::ranges::repeat_view<A>,
                                             cuda::std::piecewise_construct_t,
                                             cuda::std::tuple<int, int, int>,
                                             cuda::std::tuple<cuda::std::unreachable_sentinel_t>>);
static_assert(!cuda::std::constructible_from<cuda::std::ranges::repeat_view<A>,
                                             cuda::std::piecewise_construct_t,
                                             cuda::std::tuple<B>,
                                             cuda::std::tuple<int>>);

__host__ __device__ constexpr bool test()
{
  {
    cuda::std::ranges::repeat_view<A, int> rv(cuda::std::piecewise_construct, cuda::std::tuple{}, cuda::std::tuple{3});
    assert(rv.size() == 3);
    assert(rv[0].x == 111);
    assert(rv[0].y == 222);
    assert(rv.begin() + 3 == rv.end());
  }
  {
    cuda::std::ranges::repeat_view<A> rv(
      cuda::std::piecewise_construct, cuda::std::tuple{}, cuda::std::tuple{cuda::std::unreachable_sentinel});
    assert(rv[0].x == 111);
    assert(rv[0].y == 222);
    assert(rv.begin() + 300 != rv.end());
  }
  {
    cuda::std::ranges::repeat_view<A, int> rv(
      cuda::std::piecewise_construct, cuda::std::tuple{1, 2}, cuda::std::tuple{3});
    assert(rv.size() == 3);
    assert(rv[0].x == 1);
    assert(rv[0].y == 2);
    assert(rv.begin() + 3 == rv.end());
  }
  {
    cuda::std::ranges::repeat_view<A> rv(
      cuda::std::piecewise_construct, cuda::std::tuple{1, 2}, cuda::std::tuple{cuda::std::unreachable_sentinel});
    assert(rv[0].x == 1);
    assert(rv[0].y == 2);
    assert(rv.begin() + 300 != rv.end());
  }
  {
    cuda::std::ranges::repeat_view<A, int> rv(
      cuda::std::piecewise_construct, cuda::std::tuple{B{11}}, cuda::std::tuple{3});
    assert(rv.size() == 3);
    assert(rv[0].x == 11);
    assert(rv[0].y == 12);
    assert(rv.begin() + 3 == rv.end());
  }
  {
    cuda::std::ranges::repeat_view<A> rv(
      cuda::std::piecewise_construct, cuda::std::tuple{B{10}}, cuda::std::tuple{cuda::std::unreachable_sentinel});
    assert(rv[0].x == 10);
    assert(rv[0].y == 11);
    assert(rv.begin() + 300 != rv.end());
  }

  return true;
}

int main(int, char**)
{
  test();
  // static_assert(test());

  return 0;
}
