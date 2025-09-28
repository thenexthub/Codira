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

// constexpr auto data() requires contiguous_range<R>
// constexpr auto data() const requires contiguous_range<const R>

#include <uscl/std/array>
#include <uscl/std/cassert>
#include <uscl/std/concepts>
#include <uscl/std/ranges>

#include "test_iterators.h"
#include "test_macros.h"

template <class T>
_CCCL_CONCEPT HasData = _CCCL_REQUIRES_EXPR((T), T t)(unused(t.data()));

struct ContiguousIters
{
  __host__ __device__ contiguous_iterator<int*> begin();
  __host__ __device__ sentinel_wrapper<contiguous_iterator<int*>> end();
};

struct NoData
{
  __host__ __device__ random_access_iterator<int*> begin();
  __host__ __device__ random_access_iterator<int*> end();
};

__host__ __device__ constexpr bool test()
{
  {
    using OwningView = cuda::std::ranges::owning_view<ContiguousIters>;
    static_assert(cuda::std::ranges::contiguous_range<OwningView&>);
    static_assert(!cuda::std::ranges::range<const OwningView&>); // no begin/end
    static_assert(HasData<OwningView&>);
    static_assert(HasData<OwningView&&>);
    static_assert(!HasData<const OwningView&>);
    static_assert(!HasData<const OwningView&&>);
  }
  {
    using OwningView = cuda::std::ranges::owning_view<NoData>;
    static_assert(!HasData<OwningView&>);
    static_assert(!HasData<OwningView&&>);
    static_assert(!HasData<const OwningView&>);
    static_assert(!HasData<const OwningView&&>);
  }
  {
    // Test a view.
    int a[] = {1};
    auto ov = cuda::std::ranges::owning_view(cuda::std::ranges::subrange(a, a + 1));
    assert(ov.data() == a);
    assert(cuda::std::as_const(ov).data() == a);
  }
  {
    // Test a non-view.
    cuda::std::array<int, 2> a = {1, 2};
    auto ov                    = cuda::std::ranges::owning_view(cuda::std::move(a));
    assert(ov.data() != a.data()); // because it points into the copy
    assert(cuda::std::as_const(ov).data() != a.data());
  }
  return true;
}

int main(int, char**)
{
  test();
  static_assert(test());

  return 0;
}
