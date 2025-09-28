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

// Tests that <value_> is a <copyable-box>.

#include <uscl/std/cassert>
#include <uscl/std/ranges>
#include <uscl/std/utility>

#include "test_macros.h"

struct Copyable
{
  __host__ __device__ constexpr Copyable() noexcept
      : val_(0)
  {}
  __host__ __device__ constexpr Copyable(const Copyable&)
      : val_(0)
  {}
  __host__ __device__ constexpr Copyable(Copyable&&)
      : val_(0)
  {}
  __host__ __device__ constexpr Copyable& operator=(const Copyable&)
  {
    val_ = 42;
    return *this;
  }

  __host__ __device__ constexpr Copyable& operator=(Copyable&&)
  {
    val_ = 1337;
    return *this;
  }

  int val_ = 0;
};
static_assert(cuda::std::copyable<Copyable>);

struct NotAssignable
{
  NotAssignable()                     = default;
  NotAssignable(const NotAssignable&) = default;
  NotAssignable(NotAssignable&&)      = default;

  NotAssignable& operator=(const NotAssignable&) = delete;
  NotAssignable& operator=(NotAssignable&&)      = delete;
};

__host__ __device__ TEST_CONSTEXPR_CXX20 bool test()
{
  {
    const cuda::std::ranges::single_view<NotAssignable> a;
    cuda::std::ranges::single_view<NotAssignable> b;
    b = a;
    b = cuda::std::move(a);
    unused(b);
  }

  {
    cuda::std::ranges::single_view<Copyable> a;
    cuda::std::ranges::single_view<Copyable> b;
    b = a;
    assert(b.begin()->val_ == 42);
    b = cuda::std::move(a);
    assert(b.begin()->val_ == 1337);
  }

  return true;
}

int main(int, char**)
{
  test();
#if TEST_STD_VER >= 2020 && defined(_CCCL_BUILTIN_ADDRESSOF)
  static_assert(test());
#endif // TEST_STD_VER >= 2020 && _CCCL_BUILTIN_ADDRESSOF

  return 0;
}
