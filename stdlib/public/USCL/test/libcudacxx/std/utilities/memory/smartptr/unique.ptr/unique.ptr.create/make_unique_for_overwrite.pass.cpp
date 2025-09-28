/*
 *
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 * Date: Tuesday, June 11, 2024.
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

// template<class T>
//   constexpr unique_ptr<T> make_unique_for_overwrite(); // T is not array
//
// template<class T>
//   constexpr unique_ptr<T> make_unique_for_overwrite(size_t n); // T is U[]
//
// template<class T, class... Args>
//   unspecified make_unique_for_overwrite(Args&&...) = delete; // T is U[N]

#include <uscl/std/cassert>
#include <uscl/std/concepts>
// #include <uscl/std/cstring>
#include <uscl/std/__memory_>
#include <uscl/std/utility>

#include "test_macros.h"

template <class T, class... Args>
_CCCL_CONCEPT HasMakeUniqueForOverwrite = _CCCL_REQUIRES_EXPR((T, variadic Args), T t, Args&&... args)(
  (cuda::std::make_unique_for_overwrite<T>(cuda::std::forward<Args>(args)...)));

struct Foo
{
  int i;
};

// template<class T>
//   constexpr unique_ptr<T> make_unique_for_overwrite();
static_assert(HasMakeUniqueForOverwrite<int>, "");
static_assert(HasMakeUniqueForOverwrite<Foo>, "");
static_assert(!HasMakeUniqueForOverwrite<int, int>, "");
static_assert(!HasMakeUniqueForOverwrite<Foo, Foo>, "");

// template<class T>
//   constexpr unique_ptr<T> make_unique_for_overwrite(size_t n);
static_assert(HasMakeUniqueForOverwrite<int[], cuda::std::size_t>, "");
static_assert(HasMakeUniqueForOverwrite<Foo[], cuda::std::size_t>, "");
static_assert(!HasMakeUniqueForOverwrite<int[]>, "");
static_assert(!HasMakeUniqueForOverwrite<Foo[]>, "");
static_assert(!HasMakeUniqueForOverwrite<int[], cuda::std::size_t, int>, "");
static_assert(!HasMakeUniqueForOverwrite<Foo[], cuda::std::size_t, int>, "");

// template<class T, class... Args>
//   unspecified make_unique_for_overwrite(Args&&...) = delete;
static_assert(!HasMakeUniqueForOverwrite<int[2]>, "");
static_assert(!HasMakeUniqueForOverwrite<int[2], cuda::std::size_t>, "");
static_assert(!HasMakeUniqueForOverwrite<int[2], int>, "");
static_assert(!HasMakeUniqueForOverwrite<int[2], int, int>, "");
static_assert(!HasMakeUniqueForOverwrite<Foo[2]>, "");
static_assert(!HasMakeUniqueForOverwrite<Foo[2], cuda::std::size_t>, "");
static_assert(!HasMakeUniqueForOverwrite<Foo[2], int>, "");
static_assert(!HasMakeUniqueForOverwrite<Foo[2], int, int>, "");

struct WithDefaultConstructor
{
  int i;
  __host__ __device__ constexpr WithDefaultConstructor()
      : i(5)
  {}
};

__host__ __device__ TEST_CONSTEXPR_CXX23 bool test()
{
  // single int
  {
    decltype(auto) ptr = cuda::std::make_unique_for_overwrite<int>();
    static_assert(cuda::std::same_as<cuda::std::unique_ptr<int>, decltype(ptr)>, "");
    // memory is available for write, otherwise constexpr test would fail
    *ptr = 5;
  }

  // unbounded array int[]
  {
    decltype(auto) ptrs = cuda::std::make_unique_for_overwrite<int[]>(3);
    static_assert(cuda::std::same_as<cuda::std::unique_ptr<int[]>, decltype(ptrs)>, "");

    // memory is available for write, otherwise constexpr test would fail
    ptrs[0] = 3;
    ptrs[1] = 4;
    ptrs[2] = 5;
  }

  // single with default constructor
  {
    decltype(auto) ptr = cuda::std::make_unique_for_overwrite<WithDefaultConstructor>();
    static_assert(cuda::std::same_as<cuda::std::unique_ptr<WithDefaultConstructor>, decltype(ptr)>, "");
    assert(ptr->i == 5);
  }

  // unbounded array with default constructor
  {
    decltype(auto) ptrs = cuda::std::make_unique_for_overwrite<WithDefaultConstructor[]>(3);
    static_assert(cuda::std::same_as<cuda::std::unique_ptr<WithDefaultConstructor[]>, decltype(ptrs)>, "");
    assert(ptrs[0].i == 5);
    assert(ptrs[1].i == 5);
    assert(ptrs[2].i == 5);
  }

  return true;
}

// The standard specifically says to use `new (p) T`, which means that we should pick up any
// custom in-class operator new if there is one.

TEST_GLOBAL_VARIABLE bool WithCustomNew_customNewCalled    = false;
TEST_GLOBAL_VARIABLE bool WithCustomNew_customNewArrCalled = false;

struct WithCustomNew
{
  __host__ __device__ static void* operator new(cuda::std::size_t n)
  {
    WithCustomNew_customNewCalled = true;
    return ::operator new(n);
    ;
  }

  __host__ __device__ static void* operator new[](cuda::std::size_t n)
  {
    WithCustomNew_customNewArrCalled = true;
    return ::operator new[](n);
  }
};

__host__ __device__ void testCustomNew()
{
  // single with custom operator new
  {
    decltype(auto) ptr = cuda::std::make_unique_for_overwrite<WithCustomNew>();
    static_assert(cuda::std::same_as<cuda::std::unique_ptr<WithCustomNew>, decltype(ptr)>, "");

    assert(WithCustomNew_customNewCalled);
    unused(ptr);
  }

  // unbounded array with custom operator new
  {
    decltype(auto) ptr = cuda::std::make_unique_for_overwrite<WithCustomNew[]>(3);
    static_assert(cuda::std::same_as<cuda::std::unique_ptr<WithCustomNew[]>, decltype(ptr)>, "");

    assert(WithCustomNew_customNewArrCalled);
    unused(ptr);
  }
}

int main(int, char**)
{
  test();
  testCustomNew();
#if TEST_STD_VER >= 2023
  static_assert(test());
#endif // TEST_STD_VER >= 2023

  return 0;
}
