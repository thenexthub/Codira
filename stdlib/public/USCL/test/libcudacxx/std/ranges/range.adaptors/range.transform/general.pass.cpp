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

// Some basic examples of how transform_view might be used in the wild. This is a general
// collection of sample algorithms and functions that try to mock general usage of
// this view.

#include <uscl/std/cassert>
#include <uscl/std/functional>
#include <uscl/std/inplace_vector>
#include <uscl/std/ranges>
#include <uscl/std/string_view>

#include "test_iterators.h"
#include "test_macros.h"
#include "types.h"

template <class T, class F>
_CCCL_CONCEPT ValidTransformView =
  _CCCL_REQUIRES_EXPR((T, F))(typename(typename cuda::std::ranges::transform_view<T, F>));

struct BadFunction
{};
static_assert(ValidTransformView<MoveOnlyView, PlusOne>);
static_assert(!ValidTransformView<Range, PlusOne>);
static_assert(!ValidTransformView<MoveOnlyView, BadFunction>);

struct toUpperFn
{
  __host__ __device__ constexpr char operator()(const char c) const noexcept
  {
    if (c >= 'a' && c <= 'z')
    {
      return static_cast<char>(c - 32);
    }
    return c;
  }
};

template <class R, cuda::std::enable_if_t<cuda::std::ranges::range<R>, int> = 0>
__host__ __device__ auto toUpper(R range)
{
  return cuda::std::ranges::transform_view(range, toUpperFn{});
}

template <class E1, class E2, size_t N, class Join = cuda::std::plus<E1>>
__host__ __device__ auto joinArrays(E1 (&a)[N], E2 (&b)[N], Join join = Join())
{
  return cuda::std::ranges::transform_view(a, [&a, &b, join](E1& x) {
    auto idx = (&x) - a;
    return join(x, b[idx]);
  });
}

struct NonConstView : cuda::std::ranges::view_base
{
  __host__ __device__ explicit NonConstView(int* b, int* e)
      : b_(b)
      , e_(e)
  {}
  __host__ __device__ const int* begin()
  {
    return b_;
  } // deliberately non-const
  __host__ __device__ const int* end()
  {
    return e_;
  } // deliberately non-const
  const int* b_;
  const int* e_;
};

template <class Range, class Expected>
__host__ __device__ constexpr bool equal(Range&& range, Expected&& expected)
{
  for (size_t i = 0; i < cuda::std::size(expected); ++i)
  {
    if (range[i] != expected[i])
    {
      return false;
    }
  }
  return true;
}

int main(int, char**)
{
  {
    cuda::std::inplace_vector<int, 5> vec = {1, 2, 3, 4};
    auto transformed                      = cuda::std::ranges::transform_view(vec, [](int x) {
      return x + 42;
    });
    int expected[]                        = {43, 44, 45, 46};
    assert(equal(transformed, expected));
    const auto& ct = transformed;
    assert(equal(ct, expected));
  }

  {
    // Test a view type that is not const-iterable.
    int a[]          = {1, 2, 3, 4};
    auto transformed = NonConstView(a, a + 4) | cuda::std::views::transform([](int x) {
                         return x + 42;
                       });
    int expected[4]  = {43, 44, 45, 46};
    assert(equal(transformed, expected));
  }

  {
    int a[4]     = {1, 2, 3, 4};
    int b[4]     = {4, 3, 2, 1};
    auto out     = joinArrays(a, b);
    int check[4] = {5, 5, 5, 5};
    assert(equal(out, check));
  }

  {
    cuda::std::string_view str   = "Hello, World.";
    auto upp                     = toUpper(str);
    cuda::std::string_view check = "HELLO, WORLD.";
    assert(equal(upp, check));
  }

  return 0;
}
