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

#if defined(__clang__)
#  pragma clang diagnostic ignored "-Wsign-compare"
#elif defined(__GNUC__)
#  pragma GCC diagnostic ignored "-Wsign-compare"
#elif defined(_MSC_VER)
#  pragma warning(disable : 4018 4389) // various "signed/unsigned mismatch"
#endif

// constexpr iota_view(type_identity_t<W> value, type_identity_t<Bound> bound);

#include <uscl/std/cassert>
#include <uscl/std/ranges>

#include "test_macros.h"
#include "types.h"

__host__ __device__ constexpr bool test()
{
  {
    cuda::std::ranges::iota_view<SomeInt, SomeInt> io(SomeInt(0), SomeInt(10));
    assert(cuda::std::ranges::next(io.begin(), 10) == io.end());
  }

  {
    cuda::std::ranges::iota_view<SomeInt> io(SomeInt(0), cuda::std::unreachable_sentinel);
    assert(cuda::std::ranges::next(io.begin(), 10) != io.end());
  }

  {
    cuda::std::ranges::iota_view<SomeInt, IntComparableWith<SomeInt>> io(SomeInt(0), IntComparableWith(SomeInt(10)));
    assert(cuda::std::ranges::next(io.begin(), 10) == io.end());
  }

  {
    // This is allowed only when using the constructor (not the deduction guide).
    cuda::std::ranges::iota_view<int, unsigned> signedUnsigned(0, 10);
    assert(cuda::std::ranges::next(signedUnsigned.begin(), 10) == signedUnsigned.end());
  }

  {
    // This is allowed only when using the constructor (not the deduction guide).
    cuda::std::ranges::iota_view<unsigned, int> signedUnsigned(0, 10);
    assert(cuda::std::ranges::next(signedUnsigned.begin(), 10) == signedUnsigned.end());
  }

  return true;
}

int main(int, char**)
{
  test();
  static_assert(test(), "");

  return 0;
}
