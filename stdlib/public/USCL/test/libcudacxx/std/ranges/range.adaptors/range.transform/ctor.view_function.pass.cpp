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

// constexpr transform_view(View, F);

#include <uscl/std/cassert>
#include <uscl/std/ranges>

#include "test_macros.h"

struct Range : cuda::std::ranges::view_base
{
  __host__ __device__ constexpr explicit Range(int* b, int* e)
      : begin_(b)
      , end_(e)
  {}
  __host__ __device__ constexpr int* begin() const
  {
    return begin_;
  }
  __host__ __device__ constexpr int* end() const
  {
    return end_;
  }

private:
  int* begin_;
  int* end_;
};

struct F
{
  __host__ __device__ constexpr int operator()(int i) const
  {
    return i + 100;
  }
};

__host__ __device__ constexpr bool test()
{
  int buff[] = {1, 2, 3, 4, 5, 6, 7, 8};

  {
    Range range(buff, buff + 8);
    F f{};
    cuda::std::ranges::transform_view<Range, F> view(range, f);
    assert(view[0] == 101);
    assert(view[1] == 102);
    // ...
    assert(view[7] == 108);
  }

  {
    Range range(buff, buff + 8);
    F f{};
    cuda::std::ranges::transform_view<Range, F> view = {range, f};
    assert(view[0] == 101);
    assert(view[1] == 102);
    // ...
    assert(view[7] == 108);
  }

  return true;
}

int main(int, char**)
{
  test();
#if defined(_CCCL_BUILTIN_ADDRESSOF)
  static_assert(test(), "");
#endif // _CCCL_BUILTIN_ADDRESSOF

  return 0;
}
