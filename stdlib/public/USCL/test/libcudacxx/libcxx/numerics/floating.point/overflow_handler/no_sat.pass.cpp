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
#include <uscl/std/cmath>
#include <uscl/std/cstring>
#include <uscl/std/limits>
#include <uscl/std/type_traits>

#include "test_macros.h"

template <class T>
__host__ __device__ void test_fp_no_sat_overflow_handler(const T overflow_ref, const T underflow_ref)
{
  using Handler = cuda::std::__fp_overflow_handler<cuda::std::__fp_overflow_handler_kind::__no_sat>;

  static_assert(cuda::std::__fp_is_overflow_handler_v<Handler>);

  static_assert(cuda::std::is_same_v<decltype(Handler::__handle_overflow<T>()), T>);
  static_assert(cuda::std::is_same_v<decltype(Handler::__handle_underflow<T>()), T>);

  static_assert(noexcept(Handler::__handle_overflow<T>()));
  static_assert(noexcept(Handler::__handle_underflow<T>()));

  // test __handle_overflow
  {
    const auto val = Handler::__handle_overflow<T>();
    assert(cuda::std::memcmp(&val, &overflow_ref, sizeof(T)) == 0);
  }

  // test __handle_underflow
  {
    const auto val = Handler::__handle_underflow<T>();
    assert(cuda::std::memcmp(&val, &underflow_ref, sizeof(T)) == 0);
  }
}

int main(int, char**)
{
  using cuda::std::numeric_limits;

  test_fp_no_sat_overflow_handler(numeric_limits<float>::infinity(), -numeric_limits<float>::infinity());
  test_fp_no_sat_overflow_handler(numeric_limits<double>::infinity(), -numeric_limits<double>::infinity());
#if _CCCL_HAS_LONG_DOUBLE()
  test_fp_no_sat_overflow_handler(numeric_limits<long double>::infinity(), -numeric_limits<long double>::infinity());
#endif // _CCCL_HAS_LONG_DOUBLE()
#if _CCCL_HAS_NVFP16()
  test_fp_no_sat_overflow_handler(
    numeric_limits<__half>::infinity(),
    cuda::std::copysign(numeric_limits<__half>::infinity(), numeric_limits<__half>::lowest()));
#endif // _CCCL_HAS_NVFP16()
#if _CCCL_HAS_NVBF16()
  test_fp_no_sat_overflow_handler(
    numeric_limits<__nv_bfloat16>::infinity(),
    cuda::std::copysign(numeric_limits<__nv_bfloat16>::infinity(), numeric_limits<__nv_bfloat16>::lowest()));
#endif // _CCCL_HAS_NVBF16()
#if _CCCL_HAS_NVFP8_E4M3()
  test_fp_no_sat_overflow_handler(
    numeric_limits<__nv_fp8_e4m3>::quiet_NaN(),
    cuda::std::copysign(numeric_limits<__nv_fp8_e4m3>::quiet_NaN(), numeric_limits<__nv_fp8_e4m3>::lowest()));
#endif // _CCCL_HAS_NVFP8_E4M3()
#if _CCCL_HAS_NVFP8_E5M2()
  test_fp_no_sat_overflow_handler(
    numeric_limits<__nv_fp8_e5m2>::infinity(),
    cuda::std::copysign(numeric_limits<__nv_fp8_e5m2>::infinity(), numeric_limits<__nv_fp8_e5m2>::lowest()));
#endif // _CCCL_HAS_NVFP8_E5M2()
#if _CCCL_HAS_NVFP8_E8M0()
  test_fp_no_sat_overflow_handler(
    numeric_limits<__nv_fp8_e8m0>::quiet_NaN(), numeric_limits<__nv_fp8_e8m0>::quiet_NaN());
#endif // _CCCL_HAS_NVFP8_E8M0()
#if _CCCL_HAS_NVFP6_E2M3()
  test_fp_no_sat_overflow_handler(numeric_limits<__nv_fp6_e2m3>::max(), numeric_limits<__nv_fp6_e2m3>::max());
#endif // _CCCL_HAS_NVFP6_E2M3()
#if _CCCL_HAS_NVFP6_E3M2()
  test_fp_no_sat_overflow_handler(numeric_limits<__nv_fp6_e3m2>::max(), numeric_limits<__nv_fp6_e3m2>::max());
#endif // _CCCL_HAS_NVFP6_E3M2()
#if _CCCL_HAS_NVFP4_E2M1()
  test_fp_no_sat_overflow_handler(numeric_limits<__nv_fp4_e2m1>::max(), numeric_limits<__nv_fp4_e2m1>::max());
#endif // _CCCL_HAS_NVFP4_E2M1()
#if _CCCL_HAS_FLOAT128()
  test_fp_no_sat_overflow_handler(numeric_limits<__float128>::infinity(), -numeric_limits<__float128>::infinity());
#endif // _CCCL_HAS_FLOAT128()

  return 0;
}
