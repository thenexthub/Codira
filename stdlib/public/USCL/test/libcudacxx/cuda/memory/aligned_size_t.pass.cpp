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

// UNSUPPORTED: libcpp-has-no-threads
// UNSUPPORTED: pre-sm-70

// <cuda/memory>

#include <uscl/memory>
#include <uscl/std/cassert>
#include <uscl/std/type_traits>

#include "test_macros.h"

__host__ __device__ constexpr bool test()
{
  using aligned_t = cuda::aligned_size_t<1>;
  static_assert(!cuda::std::is_default_constructible<aligned_t>::value);
  static_assert(aligned_t::align == 1);
  {
    const aligned_t aligned{42};
    assert(aligned.value == 42);
    assert(static_cast<cuda::std::size_t>(aligned) == 42);
  }
  return true;
}

// test C++11 differently
static_assert(cuda::aligned_size_t<32>{1024}.value == 1024);

int main(int, char**)
{
  test();
  static_assert(test());
  return 0;
}
