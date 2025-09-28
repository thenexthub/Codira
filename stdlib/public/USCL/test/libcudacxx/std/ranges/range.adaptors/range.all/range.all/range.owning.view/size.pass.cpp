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

// constexpr auto size() requires sized_range<R>
// constexpr auto size() const requires sized_range<const R>

#include <uscl/std/array>
#include <uscl/std/cassert>
#include <uscl/std/concepts>
#include <uscl/std/ranges>

#include "test_iterators.h"
#include "test_macros.h"

template <class T>
_CCCL_CONCEPT HasSize = _CCCL_REQUIRES_EXPR((T), T t)(unused(t.size()));

struct SubtractableIters
{
  __host__ __device__ forward_iterator<int*> begin();
  __host__ __device__ sized_sentinel<forward_iterator<int*>> end();
};

struct NoSize
{
  __host__ __device__ bidirectional_iterator<int*> begin();
  __host__ __device__ bidirectional_iterator<int*> end();
};

struct SizeMember
{
  __host__ __device__ bidirectional_iterator<int*> begin();
  __host__ __device__ bidirectional_iterator<int*> end();
  __host__ __device__ int size() const;
};

__host__ __device__ constexpr bool test()
{
  {
    using OwningView = cuda::std::ranges::owning_view<SubtractableIters>;
    static_assert(cuda::std::ranges::sized_range<OwningView&>);
    static_assert(!cuda::std::ranges::range<const OwningView&>); // no begin/end
    static_assert(HasSize<OwningView&>);
    static_assert(HasSize<OwningView&&>);
    static_assert(!HasSize<const OwningView&>);
    static_assert(!HasSize<const OwningView&&>);
  }
  {
    using OwningView = cuda::std::ranges::owning_view<NoSize>;
    static_assert(!HasSize<OwningView&>);
    static_assert(!HasSize<OwningView&&>);
    static_assert(!HasSize<const OwningView&>);
    static_assert(!HasSize<const OwningView&&>);
  }
  {
    using OwningView = cuda::std::ranges::owning_view<SizeMember>;
    static_assert(cuda::std::ranges::sized_range<OwningView&>);
    static_assert(!cuda::std::ranges::range<const OwningView&>); // no begin/end
    static_assert(HasSize<OwningView&>);
    static_assert(HasSize<OwningView&&>);
    static_assert(!HasSize<const OwningView&>); // not a range, therefore no size()
    static_assert(!HasSize<const OwningView&&>);
  }
  {
    // Test an empty view.
    int a[] = {1};
    auto ov = cuda::std::ranges::owning_view(cuda::std::ranges::subrange(a, a));
    assert(ov.size() == 0);
    assert(cuda::std::as_const(ov).size() == 0);
  }
  {
    // Test a non-empty view.
    int a[] = {1};
    auto ov = cuda::std::ranges::owning_view(cuda::std::ranges::subrange(a, a + 1));
    assert(ov.size() == 1);
    assert(cuda::std::as_const(ov).size() == 1);
  }
  {
    // Test a non-view.
    cuda::std::array<int, 2> a = {1, 2};
    auto ov                    = cuda::std::ranges::owning_view(cuda::std::move(a));
    assert(ov.size() == 2);
    assert(cuda::std::as_const(ov).size() == 2);
  }
  return true;
}

int main(int, char**)
{
  test();
  static_assert(test());

  return 0;
}
