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

#include <uscl/cmath>
#include <uscl/std/cassert>
#include <uscl/std/limits>
#include <uscl/std/type_traits>
#include <uscl/std/utility>

template <class T>
__host__ __device__ constexpr void test_neg(T pos, T neg)
{
  assert(cuda::neg(pos) == neg);
  assert(cuda::neg(neg) == pos);
}

template <class T>
__host__ __device__ constexpr void test_type()
{
  static_assert(cuda::std::is_same_v<decltype(cuda::neg(T{})), T>);
  static_assert(noexcept(cuda::neg(T{})));

  test_neg<T>(0, 0);
  test_neg<T>(1, T(-1));
  test_neg<T>(4, T(-4));
  test_neg<T>(29, T(-29));
  test_neg<T>(127, T(-127));
  test_neg<T>(cuda::std::numeric_limits<T>::max(), cuda::std::numeric_limits<T>::min() + 1);
  test_neg<T>(cuda::std::numeric_limits<T>::min(), cuda::std::numeric_limits<T>::min());
}

__host__ __device__ constexpr bool test()
{
  test_type<signed char>();
  test_type<signed short>();
  test_type<signed int>();
  test_type<signed long>();
  test_type<signed long long>();
#if _CCCL_HAS_INT128()
  test_type<__int128_t>();
#endif // _CCCL_HAS_INT128()

  test_type<unsigned char>();
  test_type<unsigned short>();
  test_type<unsigned int>();
  test_type<unsigned long>();
  test_type<unsigned long long>();
#if _CCCL_HAS_INT128()
  test_type<__uint128_t>();
#endif // _CCCL_HAS_INT128()

  return true;
}

int main(int, char**)
{
  test();
  static_assert(test());
  return 0;
}
