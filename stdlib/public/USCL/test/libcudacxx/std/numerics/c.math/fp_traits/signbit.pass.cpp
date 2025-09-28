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
__host__ __device__ constexpr void test_signbit(const T pos)
{
  assert(cuda::std::signbit(pos) == false);

  if constexpr (cuda::std::numeric_limits<T>::is_signed)
  {
    if constexpr (cuda::std::numeric_limits<T>::is_integer)
    {
      // handle integer overflow when negating the minimum value
      const T neg = (pos == cuda::std::numeric_limits<T>::min()) ? cuda::std::numeric_limits<T>::max() : -pos;
      assert(cuda::std::signbit(neg) == (pos != 0));
    }
    else if constexpr (cuda::std::is_floating_point_v<T>)
    {
      assert(cuda::std::signbit(-pos) == true);
    }
    else // nvfp types
    {
      const T neg = cuda::std::copysign(pos, cuda::std::numeric_limits<T>::lowest());
      assert(cuda::std::signbit(neg) == true);
    }
  }
}

template <class T>
__host__ __device__ constexpr void test_type(float val)
{
  static_assert(cuda::std::is_same_v<bool, decltype(cuda::std::signbit(T{}))>);
  if constexpr (cuda::std::is_integral_v<T>)
  {
    // clang-cuda cannot directly cast float to the 128 bit integers. going through int is fine though
    test_signbit(static_cast<T>(static_cast<int>(val)));
  }
  else
  {
    test_signbit(cuda::std::__fp_cast<T>(val));
  }

  // __nv_fp8_e8m0 cannot represent 0
#if _CCCL_HAS_NVFP8_E8M0()
  if constexpr (!cuda::std::is_same_v<T, __nv_fp8_e8m0>)
#endif // _CCCL_HAS_NVFP8_E8M0
  {
    test_signbit(T{});
  }
  if constexpr (!cuda::std::numeric_limits<T>::is_integer)
  {
    test_signbit(cuda::std::numeric_limits<T>::min());
  }
  test_signbit(cuda::std::numeric_limits<T>::max());

  if constexpr (cuda::std::numeric_limits<T>::has_infinity)
  {
    test_signbit(cuda::std::numeric_limits<T>::infinity());
  }

  if constexpr (cuda::std::numeric_limits<T>::has_quiet_NaN)
  {
    test_signbit(cuda::std::numeric_limits<T>::quiet_NaN());
  }

  if constexpr (cuda::std::numeric_limits<T>::has_signaling_NaN)
  {
    test_signbit(cuda::std::numeric_limits<T>::signaling_NaN());
  }

  if constexpr (cuda::std::numeric_limits<T>::has_denorm)
  {
    test_signbit(cuda::std::numeric_limits<T>::denorm_min());
  }
}

__host__ __device__ constexpr bool test(float val)
{
  test_type<float>(val);
  test_type<double>(val);
#if _CCCL_HAS_LONG_DOUBLE()
  test_type<long double>(val);
#endif // _CCCL_HAS_LONG_DOUBLE()
#if _CCCL_HAS_NVFP16()
  test_type<__half>(val);
#endif // _CCCL_HAS_NVFP16()
#if _CCCL_HAS_NVBF16()
  test_type<__nv_bfloat16>(val);
#endif // _CCCL_HAS_NVBF16()
#if _CCCL_HAS_NVFP8_E4M3()
  test_type<__nv_fp8_e4m3>(val);
#endif // _CCCL_HAS_NVFP8_E4M3
#if _CCCL_HAS_NVFP8_E5M2()
  test_type<__nv_fp8_e5m2>(val);
#endif // _CCCL_HAS_NVFP8_E5M2
#if _CCCL_HAS_NVFP8_E8M0()
  test_type<__nv_fp8_e8m0>(val);
#endif // _CCCL_HAS_NVFP8_E8M0
#if _CCCL_HAS_NVFP6_E2M3()
  test_type<__nv_fp6_e2m3>(val);
#endif // _CCCL_HAS_NVFP6_E2M3
#if _CCCL_HAS_NVFP6_E3M2()
  test_type<__nv_fp6_e3m2>(val);
#endif // _CCCL_HAS_NVFP6_E3M2
#if _CCCL_HAS_NVFP4_E2M1()
  test_type<__nv_fp4_e2m1>(val);
#endif // _CCCL_HAS_NVFP4_E2M1

  test_type<signed char>(val);
  test_type<unsigned char>(val);
  test_type<signed short>(val);
  test_type<unsigned short>(val);
  test_type<signed int>(val);
  test_type<unsigned int>(val);
  test_type<signed long>(val);
  test_type<unsigned long>(val);
  test_type<signed long long>(val);
  test_type<unsigned long long>(val);
#if _CCCL_HAS_INT128()
  test_type<__int128_t>(val);
  test_type<__uint128_t>(val);
#endif // _CCCL_HAS_INT128()

  return true;
}

#if _CCCL_HAS_CONSTEXPR_BIT_CAST()
__host__ __device__ constexpr bool test_constexpr(float val)
{
  test_type<float>(val);
  test_type<double>(val);
#  if _CCCL_HAS_LONG_DOUBLE()
  test_type<long double>(val);
#  endif // _CCCL_HAS_LONG_DOUBLE()

  test_type<signed char>(val);
  test_type<unsigned char>(val);
  test_type<signed short>(val);
  test_type<unsigned short>(val);
  test_type<signed int>(val);
  test_type<unsigned int>(val);
  test_type<signed long>(val);
  test_type<unsigned long>(val);
  test_type<signed long long>(val);
  test_type<unsigned long long>(val);
#  if _CCCL_HAS_INT128()
  test_type<__int128_t>(val);
  test_type<__uint128_t>(val);
#  endif // _CCCL_HAS_INT128()

  return true;
}
#endif // _CCCL_HAS_CONSTEXPR_BIT_CAST()

int main(int, char**)
{
  volatile float val = 1.0f;
  test(val);
#if _CCCL_HAS_CONSTEXPR_BIT_CAST()
  static_assert(test_constexpr(1.0f));
#endif // _CCCL_HAS_CONSTEXPR_BIT_CAST()

  return 0;
}
