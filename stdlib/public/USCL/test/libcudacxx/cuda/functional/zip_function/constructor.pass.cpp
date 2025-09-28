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
#include <uscl/std/type_traits>

#include "test_macros.h"

struct Nothrow
{
  __host__ __device__ Nothrow() noexcept {}
};

struct NotDefaultable
{
  __host__ __device__ NotDefaultable() = delete;
  __host__ __device__ NotDefaultable(int) noexcept {}
};

struct MaybeThrowingDefault
{
  __host__ __device__ MaybeThrowingDefault() noexcept(false) {}
};

struct MaybeThrowingCopy
{
  __host__ __device__ MaybeThrowingCopy(const MaybeThrowingCopy&) noexcept(false) {}
};

struct MaybeThrowingMove
{
  __host__ __device__ MaybeThrowingMove(MaybeThrowingCopy&&) noexcept(false) {}
};

template <class Fn>
__host__ __device__ constexpr void test()
{
  using zip_function = cuda::zip_function<Fn>;
  static_assert(cuda::std::is_default_constructible_v<zip_function> == cuda::std::is_default_constructible_v<Fn>);
  static_assert(
    cuda::std::is_nothrow_default_constructible_v<zip_function> == cuda::std::is_nothrow_default_constructible_v<Fn>);

  static_assert(cuda::std::is_constructible_v<zip_function, Fn&&>);
  static_assert(cuda::std::is_constructible_v<zip_function, const Fn&>);
  static_assert(
    cuda::std::is_nothrow_constructible_v<zip_function, const Fn&> == cuda::std::is_nothrow_copy_constructible_v<Fn>);
  static_assert(
    cuda::std::is_nothrow_constructible_v<zip_function, Fn&&> == cuda::std::is_nothrow_move_constructible_v<Fn>);

  static_assert(cuda::std::is_copy_constructible_v<zip_function>);
  static_assert(cuda::std::is_move_constructible_v<zip_function>);
  static_assert(cuda::std::is_copy_assignable_v<zip_function>);
  static_assert(cuda::std::is_move_assignable_v<zip_function>);
}

__host__ __device__ constexpr void test()
{
  test<Nothrow>();
  test<NotDefaultable>();
  test<MaybeThrowingDefault>();
  test<MaybeThrowingCopy>();
  test<MaybeThrowingMove>();
}

int main(int, char**)
{
  return 0;
}
