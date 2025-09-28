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

#include <uscl/functional>
#include <uscl/std/tuple>
#include <uscl/std/utility>

#include "test_macros.h"

struct foo
{};

struct Immutable
{
  constexpr Immutable() = default;

  __host__ __device__ constexpr int operator()(const int a, const double b, foo) const noexcept
  {
    return a + 41;
  }
  __host__ __device__ constexpr int operator()(const int a, const double) const
  {
    return a + 41;
  }
  __host__ __device__ constexpr int operator()(const int a) const noexcept
  {
    return a + 41;
  }
};

struct Mutable
{
  constexpr Mutable() = default;

  __host__ __device__ constexpr int operator()(const int a, const double b, foo)
  {
    return a + 41;
  }
  __host__ __device__ constexpr int operator()(const int a, const double) noexcept
  {
    return a + 41;
  }
  __host__ __device__ constexpr int operator()(const int a)
  {
    return a + 41;
  }
};

struct Mixed
{
  constexpr Mixed() = default;

  __host__ __device__ constexpr int operator()(const int a, const double b, foo) const noexcept
  {
    return a + 41;
  }
  __host__ __device__ constexpr int operator()(const int a, const double) const
  {
    return a + 41;
  }
  __host__ __device__ constexpr int operator()(const int a) const noexcept
  {
    return a + 41;
  }

  __host__ __device__ constexpr int operator()(const int a, const double b, foo)
  {
    return a + 41;
  }
  __host__ __device__ constexpr int operator()(const int a, const double) noexcept
  {
    return a + 41;
  }
  __host__ __device__ constexpr int operator()(const int a)
  {
    return a + 41;
  }
};

template <bool IsNoexcept, class Fn, class Tuple>
__host__ __device__ constexpr void test(Fn&& fun, Tuple&& tuple)
{
  static_assert(cuda::std::is_invocable_v<Fn, Tuple>);
  static_assert(cuda::std::is_nothrow_invocable_v<Fn, Tuple> == IsNoexcept);
  assert(cuda::std::forward<Fn>(fun)(cuda::std::forward<Tuple>(tuple)) == 42);
}

__host__ __device__ constexpr bool test()
{
  using cuda::zip_function;

  cuda::std::tuple three_args{1, 3.14, foo{}};
  cuda::std::pair two_args{1, 3.14};
  cuda::std::tuple one_arg{1};
  {
    const zip_function<Immutable> fn{};
    test<true>(fn, three_args);
    test<false>(fn, two_args);
    test<true>(fn, one_arg);

    // Ensure we can also call the function with const arguments
    test<true>(fn, cuda::std::as_const(three_args));
    test<false>(fn, cuda::std::as_const(two_args));
    test<true>(fn, cuda::std::as_const(one_arg));

    // Ensure we can also call the function with prvalues
    test<true>(fn, cuda::std::tuple{1, 3.14, foo{}});
    test<false>(fn, cuda::std::pair{1, 3.14});
    test<true>(fn, cuda::std::tuple{1});
  }

  {
    zip_function<Mutable> fn{};
    test<false>(fn, three_args);
    test<true>(fn, two_args);
    test<false>(fn, one_arg);

    // Ensure we can also call the function with const arguments
    test<false>(fn, cuda::std::as_const(three_args));
    test<true>(fn, cuda::std::as_const(two_args));
    test<false>(fn, cuda::std::as_const(one_arg));

    // Ensure we can also call the function with prvalues
    test<false>(fn, cuda::std::tuple{1, 3.14, foo{}});
    test<true>(fn, cuda::std::pair{1, 3.14});
    test<false>(fn, cuda::std::tuple{1});
  }

  { // Ensure that we properly dispatch to the const overload then possible
    const zip_function<Mixed> const_fn{};
    test<true>(const_fn, three_args);
    test<false>(const_fn, two_args);
    test<true>(const_fn, one_arg);

    // Ensure we can also call the function with const arguments
    test<true>(const_fn, cuda::std::as_const(three_args));
    test<false>(const_fn, cuda::std::as_const(two_args));
    test<true>(const_fn, cuda::std::as_const(one_arg));

    // Ensure we can also call the function with prvalues
    test<true>(const_fn, cuda::std::tuple{1, 3.14, foo{}});
    test<false>(const_fn, cuda::std::pair{1, 3.14});
    test<true>(const_fn, cuda::std::tuple{1});

    zip_function<Mixed> fn{};
    test<false>(fn, three_args);
    test<true>(fn, two_args);
    test<false>(fn, one_arg);

    // Ensure we can also call the function with const arguments
    test<false>(fn, cuda::std::as_const(three_args));
    test<true>(fn, cuda::std::as_const(two_args));
    test<false>(fn, cuda::std::as_const(one_arg));

    // Ensure we can also call the function with prvalues
    test<false>(fn, cuda::std::tuple{1, 3.14, foo{}});
    test<true>(fn, cuda::std::pair{1, 3.14});
    test<false>(fn, cuda::std::tuple{1});
  }
  return true;
};

int main(int, char**)
{
  assert(test());
  static_assert(test());
  return 0;
}
