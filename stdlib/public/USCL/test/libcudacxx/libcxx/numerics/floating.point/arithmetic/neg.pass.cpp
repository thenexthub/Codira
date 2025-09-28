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

// ADDITIONAL_COMPILE_OPTIONS_HOST: -fext-numeric-literals
// ADDITIONAL_COMPILE_DEFINITIONS: CCCL_GCC_HAS_EXTENDED_NUMERIC_LITERALS

#include <uscl/std/__floating_point/fp.h>
#include <uscl/std/cassert>
#include <uscl/std/type_traits>

template <class T>
__host__ __device__ constexpr void test_fp_neg()
{
  static_assert(cuda::std::is_same_v<decltype(cuda::std::__fp_neg(T{})), T>);
  static_assert(noexcept(cuda::std::__fp_neg(T{})));

  // todo: implement test once __fp_cast is implemented
}

__host__ __device__ constexpr bool test()
{
  // standard floating point types
  test_fp_neg<float>();
  test_fp_neg<double>();
#if _CCCL_HAS_LONG_DOUBLE()
  test_fp_neg<long double>();
#endif // _CCCL_HAS_LONG_DOUBLE()

  // extended nvidia floating point types
#if _CCCL_HAS_NVFP16()
  test_fp_neg<__half>();
#endif // _CCCL_HAS_NVFP16()
#if _CCCL_HAS_NVBF16()
  test_fp_neg<__nv_bfloat16>();
#endif // _CCCL_HAS_NVBF16()
#if _CCCL_HAS_NVFP8_E4M3()
  test_fp_neg<__nv_fp8_e4m3>();
#endif // _CCCL_HAS_NVFP8_E4M3()
#if _CCCL_HAS_NVFP8_E5M2()
  test_fp_neg<__nv_fp8_e5m2>();
#endif // _CCCL_HAS_NVFP8_E5M2()
#if _CCCL_HAS_NVFP8_E8M0()
  test_fp_neg<__nv_fp8_e8m0>();
#endif // _CCCL_HAS_NVFP8_E8M0()
#if _CCCL_HAS_NVFP6_E2M3()
  test_fp_neg<__nv_fp6_e2m3>();
#endif // _CCCL_HAS_NVFP6_E2M3()
#if _CCCL_HAS_NVFP6_E3M2()
  test_fp_neg<__nv_fp6_e3m2>();
#endif // _CCCL_HAS_NVFP6_E3M2()
#if _CCCL_HAS_NVFP4_E2M1()
  test_fp_neg<__nv_fp4_e2m1>();
#endif // _CCCL_HAS_NVFP4_E2M1()

  // extended compiler floating point types
#if _CCCL_HAS_FLOAT128()
  test_fp_neg<__float128>();
#endif // _CCCL_HAS_FLOAT128()

  // extended cccl floating point types
  test_fp_neg<cuda::std::__cccl_fp<cuda::std::__fp_format::__binary16>>();
  test_fp_neg<cuda::std::__cccl_fp<cuda::std::__fp_format::__binary32>>();
  test_fp_neg<cuda::std::__cccl_fp<cuda::std::__fp_format::__binary64>>();
#if _CCCL_HAS_INT128()
  test_fp_neg<cuda::std::__cccl_fp<cuda::std::__fp_format::__binary128>>();
#endif // _CCCL_HAS_INT128()
  test_fp_neg<cuda::std::__cccl_fp<cuda::std::__fp_format::__bfloat16>>();
#if _CCCL_HAS_INT128()
  test_fp_neg<cuda::std::__cccl_fp<cuda::std::__fp_format::__fp80_x86>>();
#endif // _CCCL_HAS_INT128()
  test_fp_neg<cuda::std::__cccl_fp<cuda::std::__fp_format::__fp8_nv_e4m3>>();
  test_fp_neg<cuda::std::__cccl_fp<cuda::std::__fp_format::__fp8_nv_e5m2>>();
  test_fp_neg<cuda::std::__cccl_fp<cuda::std::__fp_format::__fp8_nv_e8m0>>();
  test_fp_neg<cuda::std::__cccl_fp<cuda::std::__fp_format::__fp6_nv_e2m3>>();
  test_fp_neg<cuda::std::__cccl_fp<cuda::std::__fp_format::__fp6_nv_e3m2>>();
  test_fp_neg<cuda::std::__cccl_fp<cuda::std::__fp_format::__fp4_nv_e2m1>>();

  return true;
}

int main(int, char**)
{
  test();
  static_assert(test());
  return 0;
}
