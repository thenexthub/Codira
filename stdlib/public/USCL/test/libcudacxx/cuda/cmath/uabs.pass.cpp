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

template <class T, class U>
__host__ __device__ constexpr void test_uabs(T input, U ref)
{
  assert(cuda::uabs(input) == ref);
}

template <class T>
__host__ __device__ constexpr void test_type()
{
  using U = cuda::std::make_unsigned_t<T>;

  static_assert(cuda::std::is_same_v<decltype(cuda::uabs(T{})), U>);
  static_assert(noexcept(cuda::uabs(T{})));

  test_uabs(T(0), U(0));
  test_uabs(T(1), U(1));
  test_uabs(T(100), U(100));
  if constexpr (cuda::std::is_signed_v<T>)
  {
    test_uabs(T(-1), U(1));
    test_uabs(T(-100), U(100));
    test_uabs(T(cuda::std::numeric_limits<T>::min() + 1), static_cast<U>(cuda::std::numeric_limits<T>::max()));
    test_uabs(cuda::std::numeric_limits<T>::min(), U(static_cast<U>(cuda::std::numeric_limits<T>::max()) + 1));
  }
  test_uabs(cuda::std::numeric_limits<T>::max(), static_cast<U>(cuda::std::numeric_limits<T>::max()));
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
