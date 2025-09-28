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

// clang-format off
#include <disable_nvfp_conversions_and_operators.h>
// clang-format on

#include <uscl/std/__floating_point/fp.h>
#include <uscl/std/cassert>
#include <uscl/std/cstring>

#include "test_macros.h"

template <class To, class From>
__host__ __device__ void test_fp_cast()
{
  // 1.0 should be representable by all floating point types
  const From in     = cuda::std::__fp_cast<From>(1.f);
  const To expected = cuda::std::__fp_cast<To>(1.f);

  static_assert(cuda::std::is_same_v<To, decltype(cuda::std::__fp_cast<To>(From{}))>);
  const To out = cuda::std::__fp_cast<To>(in);
  assert(cuda::std::memcmp(&out, &expected, sizeof(To)) == 0);
}

template <class T>
__host__ __device__ void test_fp_cast()
{
  test_fp_cast<T, float>();
  test_fp_cast<T, double>();
#if _CCCL_HAS_LONG_DOUBLE()
  test_fp_cast<T, long double>();
#endif // _CCCL_HAS_LONG_DOUBLE()
#if _CCCL_HAS_NVFP16()
  test_fp_cast<T, __half>();
#endif // _CCCL_HAS_NVFP16()
#if _CCCL_HAS_NVBF16()
  test_fp_cast<T, __nv_bfloat16>();
#endif // _CCCL_HAS_NVBF16()
#if _CCCL_HAS_NVFP8_E4M3()
  test_fp_cast<T, __nv_fp8_e4m3>();
#endif // _CCCL_HAS_NVFP8_E4M3()
#if _CCCL_HAS_NVFP8_E5M2()
  test_fp_cast<T, __nv_fp8_e5m2>();
#endif // _CCCL_HAS_NVFP8_E5M2()
#if _CCCL_HAS_NVFP8_E8M0()
  test_fp_cast<T, __nv_fp8_e8m0>();
#endif // _CCCL_HAS_NVFP8_E8M0()
#if _CCCL_HAS_NVFP6_E2M3()
  test_fp_cast<T, __nv_fp6_e2m3>();
#endif // _CCCL_HAS_NVFP6_E2M3()
#if _CCCL_HAS_NVFP6_E3M2()
  test_fp_cast<T, __nv_fp6_e3m2>();
#endif // _CCCL_HAS_NVFP6_E3M2()
#if _CCCL_HAS_NVFP4_E2M1()
  test_fp_cast<T, __nv_fp4_e2m1>();
#endif // _CCCL_HAS_NVFP4_E2M1()
}

__host__ __device__ bool test()
{
  test_fp_cast<float>();
  test_fp_cast<double>();
#if _CCCL_HAS_LONG_DOUBLE()
  test_fp_cast<long double>();
#endif // _CCCL_HAS_LONG_DOUBLE()
#if _CCCL_HAS_NVFP16()
  test_fp_cast<__half>();
#endif // _CCCL_HAS_NVFP16()
#if _CCCL_HAS_NVBF16()
  test_fp_cast<__nv_bfloat16>();
#endif // _CCCL_HAS_NVBF16()
#if _CCCL_HAS_NVFP8_E4M3()
  test_fp_cast<__nv_fp8_e4m3>();
#endif // _CCCL_HAS_NVFP8_E4M3()
#if _CCCL_HAS_NVFP8_E5M2()
  test_fp_cast<__nv_fp8_e5m2>();
#endif // _CCCL_HAS_NVFP8_E5M2()
#if _CCCL_HAS_NVFP8_E8M0()
  test_fp_cast<__nv_fp8_e8m0>();
#endif // _CCCL_HAS_NVFP8_E8M0()
#if _CCCL_HAS_NVFP6_E2M3()
  test_fp_cast<__nv_fp6_e2m3>();
#endif // _CCCL_HAS_NVFP6_E2M3()
#if _CCCL_HAS_NVFP6_E3M2()
  test_fp_cast<__nv_fp6_e3m2>();
#endif // _CCCL_HAS_NVFP6_E3M2()
#if _CCCL_HAS_NVFP4_E2M1()
  test_fp_cast<__nv_fp4_e2m1>();
#endif // _CCCL_HAS_NVFP4_E2M1()

  return true;
}

int main(int, char**)
{
  test();
  return 0;
}
