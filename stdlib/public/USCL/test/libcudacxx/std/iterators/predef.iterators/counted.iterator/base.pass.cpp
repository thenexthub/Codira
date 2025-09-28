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

// constexpr const I& base() const &;
// constexpr I base() &&;

#include <uscl/std/iterator>

#include "test_iterators.h"
#include "test_macros.h"

struct InputOrOutputArchetype
{
  using difference_type = int;

  int* ptr;

  __host__ __device__ int operator*()
  {
    return *ptr;
  }
  __host__ __device__ void operator++(int)
  {
    ++ptr;
  }
  __host__ __device__ InputOrOutputArchetype& operator++()
  {
    ++ptr;
    return *this;
  }
};

__host__ __device__ constexpr bool test()
{
  int buffer[8] = {1, 2, 3, 4, 5, 6, 7, 8};

  {
    cuda::std::counted_iterator iter(cpp20_input_iterator<int*>{buffer}, 8);
    assert(base(iter.base()) == buffer);
    assert(base(cuda::std::move(iter).base()) == buffer);

    static_assert(noexcept(iter.base()));
    static_assert(cuda::std::is_same_v<decltype(iter.base()), const cpp20_input_iterator<int*>&>);
    static_assert(cuda::std::is_same_v<decltype(cuda::std::move(iter).base()), cpp20_input_iterator<int*>>);
  }

  {
    cuda::std::counted_iterator iter(forward_iterator<int*>{buffer}, 8);
    assert(base(iter.base()) == buffer);
    assert(base(cuda::std::move(iter).base()) == buffer);

    static_assert(cuda::std::is_same_v<decltype(iter.base()), const forward_iterator<int*>&>);
    static_assert(cuda::std::is_same_v<decltype(cuda::std::move(iter).base()), forward_iterator<int*>>);
  }

  {
    cuda::std::counted_iterator iter(contiguous_iterator<int*>{buffer}, 8);
    assert(base(iter.base()) == buffer);
    assert(base(cuda::std::move(iter).base()) == buffer);

    static_assert(cuda::std::is_same_v<decltype(iter.base()), const contiguous_iterator<int*>&>);
    static_assert(cuda::std::is_same_v<decltype(cuda::std::move(iter).base()), contiguous_iterator<int*>>);
  }

  {
    cuda::std::counted_iterator iter(InputOrOutputArchetype{buffer}, 6);
    assert(iter.base().ptr == buffer);
    assert(cuda::std::move(iter).base().ptr == buffer);

    static_assert(cuda::std::is_same_v<decltype(iter.base()), const InputOrOutputArchetype&>);
    static_assert(cuda::std::is_same_v<decltype(cuda::std::move(iter).base()), InputOrOutputArchetype>);
  }

  {
    const cuda::std::counted_iterator iter(cpp20_input_iterator<int*>{buffer}, 8);
    assert(base(iter.base()) == buffer);
    assert(base(cuda::std::move(iter).base()) == buffer);

    static_assert(cuda::std::is_same_v<decltype(iter.base()), const cpp20_input_iterator<int*>&>);
    static_assert(cuda::std::is_same_v<decltype(cuda::std::move(iter).base()), const cpp20_input_iterator<int*>&>);
  }

  {
    const cuda::std::counted_iterator iter(forward_iterator<int*>{buffer}, 7);
    assert(base(iter.base()) == buffer);
    assert(base(cuda::std::move(iter).base()) == buffer);

    static_assert(cuda::std::is_same_v<decltype(iter.base()), const forward_iterator<int*>&>);
    static_assert(cuda::std::is_same_v<decltype(cuda::std::move(iter).base()), const forward_iterator<int*>&>);
  }

  {
    const cuda::std::counted_iterator iter(contiguous_iterator<int*>{buffer}, 6);
    assert(base(iter.base()) == buffer);
    assert(base(cuda::std::move(iter).base()) == buffer);

    static_assert(cuda::std::is_same_v<decltype(iter.base()), const contiguous_iterator<int*>&>);
    static_assert(cuda::std::is_same_v<decltype(cuda::std::move(iter).base()), const contiguous_iterator<int*>&>);
  }

  {
    const cuda::std::counted_iterator iter(InputOrOutputArchetype{buffer}, 6);
    assert(iter.base().ptr == buffer);
    assert(cuda::std::move(iter).base().ptr == buffer);

    static_assert(cuda::std::is_same_v<decltype(iter.base()), const InputOrOutputArchetype&>);
    static_assert(cuda::std::is_same_v<decltype(cuda::std::move(iter).base()), const InputOrOutputArchetype&>);
  }

  return true;
}

int main(int, char**)
{
  test();
  static_assert(test());

  return 0;
}
