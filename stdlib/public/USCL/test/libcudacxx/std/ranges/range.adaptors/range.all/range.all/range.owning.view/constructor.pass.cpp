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

// owning_view() requires default_initializable<R> = default;
// constexpr owning_view(R&& t);

#include <uscl/std/cassert>
#include <uscl/std/concepts>
#include <uscl/std/ranges>
#include <uscl/std/type_traits>
#include <uscl/std/utility>

#include "test_macros.h"

struct DefaultConstructible
{
  int i;
  __host__ __device__ constexpr explicit DefaultConstructible(int j = 42) noexcept(false)
      : i(j)
  {}
  __host__ __device__ int* begin() const;
  __host__ __device__ int* end() const;
};

struct NotDefaultConstructible
{
  int i;
  __host__ __device__ constexpr explicit NotDefaultConstructible(int j) noexcept(false)
      : i(j)
  {}
  __host__ __device__ int* begin() const;
  __host__ __device__ int* end() const;
};

struct MoveChecker
{
  int i;
  __host__ __device__ constexpr explicit MoveChecker(int j)
      : i(j)
  {}
  __host__ __device__ constexpr MoveChecker(MoveChecker&& v)
      : i(cuda::std::exchange(v.i, -1))
  {}
  __host__ __device__ MoveChecker& operator=(MoveChecker&&);
  __host__ __device__ int* begin() const;
  __host__ __device__ int* end() const;
};

struct NoexceptChecker
{
  __host__ __device__ int* begin() const;
  __host__ __device__ int* end() const;
};

__host__ __device__ constexpr bool test()
{
  {
    using OwningView = cuda::std::ranges::owning_view<DefaultConstructible>;
    static_assert(cuda::std::is_constructible_v<OwningView>);
    static_assert(cuda::std::default_initializable<OwningView>);
    static_assert(cuda::std::movable<OwningView>);
    static_assert(cuda::std::is_trivially_move_constructible_v<OwningView>);
    static_assert(cuda::std::is_trivially_move_assignable_v<OwningView>);
    static_assert(!cuda::std::is_copy_constructible_v<OwningView>);
    static_assert(!cuda::std::is_copy_assignable_v<OwningView>);
    static_assert(!cuda::std::is_constructible_v<OwningView, int>);
    static_assert(!cuda::std::is_constructible_v<OwningView, DefaultConstructible&>);
    static_assert(cuda::std::is_constructible_v<OwningView, DefaultConstructible&&>);
    static_assert(!cuda::std::is_convertible_v<int, OwningView>);
    static_assert(cuda::std::is_convertible_v<DefaultConstructible&&, OwningView>);
    {
      OwningView ov;
      assert(ov.base().i == 42);
    }
    {
      OwningView ov = OwningView(DefaultConstructible(1));
      assert(ov.base().i == 1);
    }
  }
  {
    using OwningView = cuda::std::ranges::owning_view<NotDefaultConstructible>;
    static_assert(!cuda::std::is_constructible_v<OwningView>);
    static_assert(!cuda::std::default_initializable<OwningView>);
    static_assert(cuda::std::movable<OwningView>);
    static_assert(cuda::std::is_trivially_move_constructible_v<OwningView>);
    static_assert(cuda::std::is_trivially_move_assignable_v<OwningView>);
    static_assert(!cuda::std::is_copy_constructible_v<OwningView>);
    static_assert(!cuda::std::is_copy_assignable_v<OwningView>);
    static_assert(!cuda::std::is_constructible_v<OwningView, int>);
    static_assert(!cuda::std::is_constructible_v<OwningView, NotDefaultConstructible&>);
    static_assert(cuda::std::is_constructible_v<OwningView, NotDefaultConstructible&&>);
    static_assert(!cuda::std::is_convertible_v<int, OwningView>);
    static_assert(cuda::std::is_convertible_v<NotDefaultConstructible&&, OwningView>);
    {
      OwningView ov = OwningView(NotDefaultConstructible(1));
      assert(ov.base().i == 1);
    }
  }
  {
    using OwningView = cuda::std::ranges::owning_view<MoveChecker>;
    static_assert(!cuda::std::is_constructible_v<OwningView>);
    static_assert(!cuda::std::default_initializable<OwningView>);
    static_assert(cuda::std::movable<OwningView>);
    static_assert(!cuda::std::is_trivially_move_constructible_v<OwningView>);
    static_assert(!cuda::std::is_trivially_move_assignable_v<OwningView>);
    static_assert(!cuda::std::is_copy_constructible_v<OwningView>);
    static_assert(!cuda::std::is_copy_assignable_v<OwningView>);
    static_assert(!cuda::std::is_constructible_v<OwningView, int>);
    static_assert(!cuda::std::is_constructible_v<OwningView, MoveChecker&>);
    static_assert(cuda::std::is_constructible_v<OwningView, MoveChecker&&>);
    static_assert(!cuda::std::is_convertible_v<int, OwningView>);
    static_assert(cuda::std::is_convertible_v<MoveChecker&&, OwningView>);
    {
      // Check that the constructor does indeed move from the target object.
      auto m        = MoveChecker(42);
      OwningView ov = OwningView(cuda::std::move(m));
      assert(ov.base().i == 42);
      assert(m.i == -1);
    }
  }
  {
    // Check that the defaulted constructors are (not) noexcept when appropriate.

    static_assert(cuda::std::is_nothrow_constructible_v<NoexceptChecker>); // therefore,
    static_assert(cuda::std::is_nothrow_constructible_v<cuda::std::ranges::owning_view<NoexceptChecker>>);

#if !TEST_COMPILER(GCC, <, 9) && !TEST_COMPILER(MSVC)
    static_assert(!cuda::std::is_nothrow_constructible_v<DefaultConstructible>); // therefore,
#endif // !TEST_COMPILER(GCC, <, 9) && !TEST_COMPILER(MSVC)
#if !TEST_CUDA_COMPILER(NVCC) && !TEST_COMPILER(NVRTC) // nvbug3910409
    static_assert(!cuda::std::is_nothrow_constructible_v<cuda::std::ranges::owning_view<DefaultConstructible>>);
#endif // !TEST_CUDA_COMPILER(NVCC) && !TEST_COMPILER(NVRTC)

    static_assert(cuda::std::is_nothrow_move_constructible_v<NoexceptChecker>); // therefore,
    static_assert(cuda::std::is_nothrow_move_constructible_v<cuda::std::ranges::owning_view<NoexceptChecker>>);
    static_assert(!cuda::std::is_nothrow_move_constructible_v<MoveChecker>); // therefore,
    static_assert(!cuda::std::is_nothrow_move_constructible_v<cuda::std::ranges::owning_view<MoveChecker>>);
  }
  return true;
}

int main(int, char**)
{
  test();
  static_assert(test());

  return 0;
}
