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

// clang-format off
#include <disable_nvfp_conversions_and_operators.h>
// clang-format on

#include <uscl/std/__floating_point/fp.h>

template <class T>
__host__ __device__ void test_std_fp()
{
  static_assert(cuda::std::__is_std_fp_v<T>);
  static_assert(!cuda::std::__is_ext_nv_fp_v<T>);
  static_assert(!cuda::std::__is_ext_compiler_fp_v<T>);
  static_assert(!cuda::std::__is_ext_cccl_fp_v<T>);
  static_assert(!cuda::std::__is_ext_fp_v<T>);
  static_assert(cuda::std::__is_fp_v<T>);

  static_assert(cuda::std::__is_std_fp_v<const T>);
  static_assert(!cuda::std::__is_ext_nv_fp_v<const T>);
  static_assert(!cuda::std::__is_ext_compiler_fp_v<const T>);
  static_assert(!cuda::std::__is_ext_cccl_fp_v<const T>);
  static_assert(!cuda::std::__is_ext_fp_v<const T>);
  static_assert(cuda::std::__is_fp_v<const T>);

  static_assert(cuda::std::__is_std_fp_v<volatile T>);
  static_assert(!cuda::std::__is_ext_nv_fp_v<volatile T>);
  static_assert(!cuda::std::__is_ext_compiler_fp_v<volatile T>);
  static_assert(!cuda::std::__is_ext_cccl_fp_v<volatile T>);
  static_assert(!cuda::std::__is_ext_fp_v<volatile T>);
  static_assert(cuda::std::__is_fp_v<volatile T>);

  static_assert(cuda::std::__is_std_fp_v<const volatile T>);
  static_assert(!cuda::std::__is_ext_nv_fp_v<const volatile T>);
  static_assert(!cuda::std::__is_ext_compiler_fp_v<const volatile T>);
  static_assert(!cuda::std::__is_ext_cccl_fp_v<const volatile T>);
  static_assert(!cuda::std::__is_ext_fp_v<const volatile T>);
  static_assert(cuda::std::__is_fp_v<const volatile T>);
}

template <class T>
__host__ __device__ void test_ext_nv_fp()
{
  static_assert(!cuda::std::__is_std_fp_v<T>);
  static_assert(cuda::std::__is_ext_nv_fp_v<T>);
  static_assert(!cuda::std::__is_ext_compiler_fp_v<T>);
  static_assert(!cuda::std::__is_ext_cccl_fp_v<T>);
  static_assert(cuda::std::__is_ext_fp_v<T>);
  static_assert(cuda::std::__is_fp_v<T>);

  static_assert(!cuda::std::__is_std_fp_v<const T>);
  static_assert(cuda::std::__is_ext_nv_fp_v<const T>);
  static_assert(!cuda::std::__is_ext_compiler_fp_v<const T>);
  static_assert(!cuda::std::__is_ext_cccl_fp_v<const T>);
  static_assert(cuda::std::__is_ext_fp_v<const T>);
  static_assert(cuda::std::__is_fp_v<const T>);

  static_assert(!cuda::std::__is_std_fp_v<volatile T>);
  static_assert(cuda::std::__is_ext_nv_fp_v<volatile T>);
  static_assert(!cuda::std::__is_ext_compiler_fp_v<volatile T>);
  static_assert(!cuda::std::__is_ext_cccl_fp_v<volatile T>);
  static_assert(cuda::std::__is_ext_fp_v<volatile T>);
  static_assert(cuda::std::__is_fp_v<volatile T>);

  static_assert(!cuda::std::__is_std_fp_v<const volatile T>);
  static_assert(cuda::std::__is_ext_nv_fp_v<const volatile T>);
  static_assert(!cuda::std::__is_ext_compiler_fp_v<const volatile T>);
  static_assert(!cuda::std::__is_ext_cccl_fp_v<const volatile T>);
  static_assert(cuda::std::__is_ext_fp_v<const volatile T>);
  static_assert(cuda::std::__is_fp_v<const volatile T>);
}

template <class T>
__host__ __device__ void test_ext_compiler_fp()
{
  static_assert(!cuda::std::__is_std_fp_v<T>);
  static_assert(!cuda::std::__is_ext_nv_fp_v<T>);
  static_assert(cuda::std::__is_ext_compiler_fp_v<T>);
  static_assert(!cuda::std::__is_ext_cccl_fp_v<T>);
  static_assert(cuda::std::__is_ext_fp_v<T>);
  static_assert(cuda::std::__is_fp_v<T>);

  static_assert(!cuda::std::__is_std_fp_v<const T>);
  static_assert(!cuda::std::__is_ext_nv_fp_v<const T>);
  static_assert(cuda::std::__is_ext_compiler_fp_v<const T>);
  static_assert(!cuda::std::__is_ext_cccl_fp_v<const T>);
  static_assert(cuda::std::__is_ext_fp_v<const T>);
  static_assert(cuda::std::__is_fp_v<const T>);

  static_assert(!cuda::std::__is_std_fp_v<volatile T>);
  static_assert(!cuda::std::__is_ext_nv_fp_v<volatile T>);
  static_assert(cuda::std::__is_ext_compiler_fp_v<volatile T>);
  static_assert(!cuda::std::__is_ext_cccl_fp_v<volatile T>);
  static_assert(cuda::std::__is_ext_fp_v<volatile T>);
  static_assert(cuda::std::__is_fp_v<volatile T>);

  static_assert(!cuda::std::__is_std_fp_v<const volatile T>);
  static_assert(!cuda::std::__is_ext_nv_fp_v<const volatile T>);
  static_assert(cuda::std::__is_ext_compiler_fp_v<const volatile T>);
  static_assert(!cuda::std::__is_ext_cccl_fp_v<const volatile T>);
  static_assert(cuda::std::__is_ext_fp_v<const volatile T>);
  static_assert(cuda::std::__is_fp_v<const volatile T>);
}

template <cuda::std::__fp_format Fmt>
__host__ __device__ void test_ext_cccl_fp()
{
  using T = cuda::std::__cccl_fp<Fmt>;

  static_assert(!cuda::std::__is_std_fp_v<T>);
  static_assert(!cuda::std::__is_ext_nv_fp_v<T>);
  static_assert(!cuda::std::__is_ext_compiler_fp_v<T>);
  static_assert(cuda::std::__is_ext_cccl_fp_v<T>);
  static_assert(cuda::std::__is_ext_fp_v<T>);
  static_assert(cuda::std::__is_fp_v<T>);

  static_assert(!cuda::std::__is_std_fp_v<const T>);
  static_assert(!cuda::std::__is_ext_nv_fp_v<const T>);
  static_assert(!cuda::std::__is_ext_compiler_fp_v<const T>);
  static_assert(cuda::std::__is_ext_cccl_fp_v<const T>);
  static_assert(cuda::std::__is_ext_fp_v<const T>);
  static_assert(cuda::std::__is_fp_v<const T>);

  static_assert(!cuda::std::__is_std_fp_v<volatile T>);
  static_assert(!cuda::std::__is_ext_nv_fp_v<volatile T>);
  static_assert(!cuda::std::__is_ext_compiler_fp_v<volatile T>);
  static_assert(cuda::std::__is_ext_cccl_fp_v<volatile T>);
  static_assert(cuda::std::__is_ext_fp_v<volatile T>);
  static_assert(cuda::std::__is_fp_v<volatile T>);

  static_assert(!cuda::std::__is_std_fp_v<const volatile T>);
  static_assert(!cuda::std::__is_ext_nv_fp_v<const volatile T>);
  static_assert(!cuda::std::__is_ext_compiler_fp_v<const volatile T>);
  static_assert(cuda::std::__is_ext_cccl_fp_v<const volatile T>);
  static_assert(cuda::std::__is_ext_fp_v<const volatile T>);
  static_assert(cuda::std::__is_fp_v<const volatile T>);
}

int main(int, char**)
{
  test_std_fp<float>();
  test_std_fp<double>();
#if _CCCL_HAS_LONG_DOUBLE()
  test_std_fp<long double>();
#endif // _CCCL_HAS_LONG_DOUBLE()

#if _CCCL_HAS_NVFP16()
  test_ext_nv_fp<__half>();
#endif // _CCCL_HAS_NVFP16()
#if _CCCL_HAS_NVBF16()
  test_ext_nv_fp<__nv_bfloat16>();
#endif // _CCCL_HAS_NVBF16()
#if _CCCL_HAS_NVFP8_E4M3()
  test_ext_nv_fp<__nv_fp8_e4m3>();
#endif // _CCCL_HAS_NVFP8_E4M3()
#if _CCCL_HAS_NVFP8_E5M2()
  test_ext_nv_fp<__nv_fp8_e5m2>();
#endif // _CCCL_HAS_NVFP8_E5M2()
#if _CCCL_HAS_NVFP8_E8M0()
  test_ext_nv_fp<__nv_fp8_e8m0>();
#endif // _CCCL_HAS_NVFP8_E8M0()
#if _CCCL_HAS_NVFP6_E2M3()
  test_ext_nv_fp<__nv_fp6_e2m3>();
#endif // _CCCL_HAS_NVFP6_E2M3()
#if _CCCL_HAS_NVFP6_E3M2()
  test_ext_nv_fp<__nv_fp6_e3m2>();
#endif // _CCCL_HAS_NVFP6_E3M2()
#if _CCCL_HAS_NVFP4_E2M1()
  test_ext_nv_fp<__nv_fp4_e2m1>();
#endif // _CCCL_HAS_NVFP4_E2M1()

#if _CCCL_HAS_FLOAT128()
  test_ext_compiler_fp<__float128>();
#endif // _CCCL_HAS_FLOAT128()

  test_ext_cccl_fp<cuda::std::__fp_format::__binary16>();
  test_ext_cccl_fp<cuda::std::__fp_format::__binary32>();
  test_ext_cccl_fp<cuda::std::__fp_format::__binary64>();
  test_ext_cccl_fp<cuda::std::__fp_format::__binary128>();
  test_ext_cccl_fp<cuda::std::__fp_format::__fp80_x86>();
  test_ext_cccl_fp<cuda::std::__fp_format::__bfloat16>();
  test_ext_cccl_fp<cuda::std::__fp_format::__fp8_nv_e4m3>();
  test_ext_cccl_fp<cuda::std::__fp_format::__fp8_nv_e5m2>();
  test_ext_cccl_fp<cuda::std::__fp_format::__fp8_nv_e8m0>();
  test_ext_cccl_fp<cuda::std::__fp_format::__fp6_nv_e2m3>();
  test_ext_cccl_fp<cuda::std::__fp_format::__fp6_nv_e3m2>();
  test_ext_cccl_fp<cuda::std::__fp_format::__fp4_nv_e2m1>();

  return 0;
}
