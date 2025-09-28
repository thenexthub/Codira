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

template <class T, class In, class Ref>
__host__ __device__ constexpr void test_isqrt(In input, Ref ref)
{
  if (cuda::std::in_range<T>(input))
  {
    assert(cuda::isqrt(static_cast<T>(input)) == static_cast<T>(ref));
  }
}

template <class T>
__host__ __device__ constexpr void test_type()
{
  static_assert(cuda::std::is_same_v<decltype(cuda::isqrt(T{})), T>);
  static_assert(noexcept(cuda::isqrt(T{})));

  test_isqrt<T>(0, 0);
  test_isqrt<T>(1, 1);
  test_isqrt<T>(2, 1);
  test_isqrt<T>(4, 2);
  test_isqrt<T>(6, 2);
  test_isqrt<T>(43, 6);
  test_isqrt<T>(70, 8);
  test_isqrt<T>(99, 9);
  test_isqrt<T>(100, 10);
  test_isqrt<T>(2115, 45);
  test_isqrt<T>(2116, 46);
  test_isqrt<T>(9801, 99);
  test_isqrt<T>(2147483647, 46340);
  test_isqrt<T>(9223372036854775807, 3037000499);
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
