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

#include <uscl/functional>

#include "test_macros.h"

template <typename OpT, typename T>
__host__ __device__ constexpr bool test_op(const T lhs, const T rhs, const T expected)
{
  return (OpT{}(lhs, rhs) == expected) && (OpT{}(lhs, rhs) == OpT{}(rhs, lhs));
}

template <typename T>
__host__ __device__ constexpr bool test(T lhs, T rhs, T expected)
{
  return test_op<cuda::minimum<T>>(lhs, rhs, expected) && //
         test_op<cuda::minimum<>>(lhs, rhs, expected) && //
         test_op<cuda::minimum<void>>(lhs, rhs, expected);
}

__host__ __device__ constexpr bool test()
{
  return test<int>(0, 1, 0) && //
         test<int>(1, 0, 0) && //
         test<int>(0, 0, 0) && //
         test<int>(-1, 1, -1) && //
         test<char>('a', 'b', 'a');
}

int main(int, char**)
{
  test();
  static_assert(test(), "");
  return 0;
}
