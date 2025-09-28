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

// constexpr bool empty() requires requires { ranges::empty(r_); }
// constexpr bool empty() const requires requires { ranges::empty(r_); }

#include <uscl/std/array>
#include <uscl/std/cassert>
#include <uscl/std/concepts>
#include <uscl/std/ranges>

#include "test_iterators.h"
#include "test_macros.h"

template <class T>
_CCCL_CONCEPT HasEmpty = _CCCL_REQUIRES_EXPR((T), T t)(unused(t.empty()));

struct ComparableIters
{
  __host__ __device__ forward_iterator<int*> begin();
  __host__ __device__ forward_iterator<int*> end();
};

struct NoEmpty
{
  __host__ __device__ cpp20_input_iterator<int*> begin();
  __host__ __device__ sentinel_wrapper<cpp20_input_iterator<int*>> end();
};

struct EmptyMember
{
  __host__ __device__ cpp20_input_iterator<int*> begin();
  __host__ __device__ sentinel_wrapper<cpp20_input_iterator<int*>> end();
  __host__ __device__ bool empty() const;
};

__host__ __device__ constexpr bool test()
{
  {
    using OwningView = cuda::std::ranges::owning_view<ComparableIters>;
    static_assert(HasEmpty<OwningView&>);
    static_assert(HasEmpty<OwningView&&>);
    static_assert(!HasEmpty<const OwningView&>);
    static_assert(!HasEmpty<const OwningView&&>);
  }
  {
    static_assert(cuda::std::ranges::range<NoEmpty&>);
    static_assert(!cuda::std::invocable<decltype(cuda::std::ranges::empty), NoEmpty&>);
    static_assert(!cuda::std::ranges::range<const NoEmpty&>); // no begin/end
    static_assert(!cuda::std::invocable<decltype(cuda::std::ranges::empty), const NoEmpty&>);
    using OwningView = cuda::std::ranges::owning_view<NoEmpty>;
    static_assert(!HasEmpty<OwningView&>);
    static_assert(!HasEmpty<OwningView&&>);
    static_assert(!HasEmpty<const OwningView&>);
    static_assert(!HasEmpty<const OwningView&&>);
  }
  {
    static_assert(cuda::std::ranges::range<EmptyMember&>);
    static_assert(cuda::std::invocable<decltype(cuda::std::ranges::empty), EmptyMember&>);
    static_assert(!cuda::std::ranges::range<const EmptyMember&>); // no begin/end
    static_assert(cuda::std::invocable<decltype(cuda::std::ranges::empty), const EmptyMember&>);
    using OwningView = cuda::std::ranges::owning_view<EmptyMember>;
    static_assert(cuda::std::ranges::range<OwningView&>);
    static_assert(!cuda::std::ranges::range<const OwningView&>); // no begin/end
    static_assert(HasEmpty<OwningView&>);
    static_assert(HasEmpty<OwningView&&>);
    static_assert(HasEmpty<const OwningView&>); // but it still has empty()
    static_assert(HasEmpty<const OwningView&&>);
  }
  {
    // Test an empty view.
    int a[] = {1};
    auto ov = cuda::std::ranges::owning_view(cuda::std::ranges::subrange(a, a));
    assert(ov.empty());
    assert(cuda::std::as_const(ov).empty());
  }
  {
    // Test a non-empty view.
    int a[] = {1};
    auto ov = cuda::std::ranges::owning_view(cuda::std::ranges::subrange(a, a + 1));
    assert(!ov.empty());
    assert(!cuda::std::as_const(ov).empty());
  }
  {
    // Test a non-view.
    cuda::std::array<int, 2> a = {1, 2};
    auto ov                    = cuda::std::ranges::owning_view(cuda::std::move(a));
    assert(!ov.empty());
    assert(!cuda::std::as_const(ov).empty());
  }
  return true;
}

int main(int, char**)
{
  test();
  static_assert(test());

  return 0;
}
