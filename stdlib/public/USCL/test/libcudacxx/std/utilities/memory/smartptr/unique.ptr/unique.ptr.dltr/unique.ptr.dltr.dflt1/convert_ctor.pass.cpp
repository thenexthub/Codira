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
// <memory>

// default_delete[]

// template <class U>
//   constexpr default_delete(const default_delete<U[]>&); // constexpr since C++23
//
// This constructor shall not participate in overload resolution unless
//   U(*)[] is convertible to T(*)[].

#include <uscl/std/__memory_>
#include <uscl/std/cassert>

#include "test_macros.h"

__host__ __device__ TEST_CONSTEXPR_CXX23 bool test()
{
  cuda::std::default_delete<int[]> d1;
  cuda::std::default_delete<const int[]> d2 = d1;
  unused(d2);

  return true;
}

int main(int, char**)
{
  test();
#if TEST_STD_VER >= 2023
  static_assert(test());
#endif // TEST_STD_VER >= 2023

  return 0;
}
