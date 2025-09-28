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

// <cmath>

// clang-format off
#include <disable_nvfp_conversions_and_operators.h>
// clang-format on

#include <uscl/std/cassert>
#include <uscl/std/cmath>
#include <uscl/std/limits>
#include <uscl/std/type_traits>
#include <uscl/type_traits>

#include "test_macros.h"

template <class T>
__host__ __device__ constexpr void test_fpclassify(T val, int expected)
{
  assert(cuda::std::fpclassify(val) == expected);

  if constexpr (cuda::std::numeric_limits<T>::is_signed)
  {
    T neg{};

    if constexpr (cuda::std::numeric_limits<T>::is_integer)
    {
      // handle integer overflow when negating the minimum value
      neg = (val == cuda::std::numeric_limits<T>::min()) ? cuda::std::numeric_limits<T>::max() : -val;
    }
    else if constexpr (cuda::std::is_floating_point_v<T>)
    {
      neg = -val;
    }
    else // nvfp types
    {
      neg = cuda::std::copysign(val, cuda::std::numeric_limits<T>::lowest());
    }

    assert(cuda::std::fpclassify(neg) == expected);
  }
}

template <class T, cuda::std::enable_if_t<cuda::is_floating_point_v<T>, int> = 0>
__host__ __device__ constexpr void test_type()
{
  static_assert(cuda::std::is_same_v<int, decltype(cuda::std::fpclassify(T{}))>);

  // __nv_fp8_e8m0 cannot represent 0
#if _CCCL_HAS_NVFP8_E8M0()
  if constexpr (!cuda::std::is_same_v<T, __nv_fp8_e8m0>)
#endif // _CCCL_HAS_NVFP8_E8M0
  {
    test_fpclassify(T{}, FP_ZERO);
  }
  test_fpclassify(cuda::std::numeric_limits<T>::min(), FP_NORMAL);
  test_fpclassify(cuda::std::numeric_limits<T>::max(), FP_NORMAL);

  if constexpr (cuda::std::numeric_limits<T>::has_infinity)
  {
    test_fpclassify(cuda::std::numeric_limits<T>::infinity(), FP_INFINITE);
  }

  if constexpr (cuda::std::numeric_limits<T>::has_quiet_NaN)
  {
    test_fpclassify(cuda::std::numeric_limits<T>::quiet_NaN(), FP_NAN);
  }

  if constexpr (cuda::std::numeric_limits<T>::has_signaling_NaN)
  {
    test_fpclassify(cuda::std::numeric_limits<T>::signaling_NaN(), FP_NAN);
  }

  if constexpr (cuda::std::numeric_limits<T>::has_denorm)
  {
    // fixme: behaviour of subnormal values depends on the FP mode, may result in FP_ZERO
    // test_fpclassify(cuda::std::numeric_limits<T>::denorm_min(), FP_SUBNORMAL);
  }
}

template <class T, cuda::std::enable_if_t<cuda::std::is_integral_v<T>, int> = 0>
__host__ __device__ constexpr void test_type()
{
  static_assert(cuda::std::is_same_v<int, decltype(cuda::std::fpclassify(T{}))>);

  test_fpclassify(T(1), FP_NORMAL);
  test_fpclassify(T(0), FP_ZERO);
  test_fpclassify(cuda::std::numeric_limits<T>::min(), cuda::std::numeric_limits<T>::is_signed ? FP_NORMAL : FP_ZERO);
  test_fpclassify(cuda::std::numeric_limits<T>::max(), FP_NORMAL);
}

__host__ __device__ constexpr bool test()
{
  test_type<float>();
  test_type<double>();
#if _CCCL_HAS_LONG_DOUBLE()
  test_type<long double>();
#endif // _CCCL_HAS_LONG_DOUBLE()
#if _CCCL_HAS_NVFP16()
  test_type<__half>();
#endif // _CCCL_HAS_NVFP16()
#if _CCCL_HAS_NVBF16()
  test_type<__nv_bfloat16>();
#endif // _CCCL_HAS_NVBF16()
#if _CCCL_HAS_NVFP8_E4M3()
  test_type<__nv_fp8_e4m3>();
#endif // _CCCL_HAS_NVFP8_E4M3
#if _CCCL_HAS_NVFP8_E5M2()
  test_type<__nv_fp8_e5m2>();
#endif // _CCCL_HAS_NVFP8_E5M2
#if _CCCL_HAS_NVFP8_E8M0()
  test_type<__nv_fp8_e8m0>();
#endif // _CCCL_HAS_NVFP8_E8M0
#if _CCCL_HAS_NVFP6_E2M3()
  test_type<__nv_fp6_e2m3>();
#endif // _CCCL_HAS_NVFP6_E2M3
#if _CCCL_HAS_NVFP6_E3M2()
  test_type<__nv_fp6_e3m2>();
#endif // _CCCL_HAS_NVFP6_E3M2
#if _CCCL_HAS_NVFP4_E2M1()
  test_type<__nv_fp4_e2m1>();
#endif // _CCCL_HAS_NVFP4_E2M1

  test_type<signed char>();
  test_type<unsigned char>();
  test_type<signed short>();
  test_type<unsigned short>();
  test_type<signed int>();
  test_type<unsigned int>();
  test_type<signed long>();
  test_type<unsigned long>();
  test_type<signed long long>();
  test_type<unsigned long long>();
#if _CCCL_HAS_INT128()
  test_type<__int128_t>();
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
