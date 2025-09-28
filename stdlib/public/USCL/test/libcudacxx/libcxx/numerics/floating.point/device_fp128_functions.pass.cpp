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

#include <uscl/std/__floating_point/cuda_fp_types.h>
#include <uscl/std/cassert>
#include <uscl/std/limits>

__device__ void test()
{
#if _CCCL_HAS_FLOAT128() && _CCCL_DEVICE_COMPILATION() && _CCCL_CTK_AT_LEAST(12, 8)
  __float128 dummy_f128{};
  int dummy_int{};

  assert(__nv_fp128_sqrt(1.q) == 1.q);
  assert(__nv_fp128_sin(0.q) == 0.q);
  assert(__nv_fp128_cos(0.q) == 1.q);
  assert(__nv_fp128_tan(0.q) == 0.q);
  assert(__nv_fp128_asin(0.q) == 0.q);
  assert(__nv_fp128_acos(1.q) == 0.q);
  assert(__nv_fp128_atan(0.q) == 0.q);
  assert(__nv_fp128_exp(0.q) == 1.q);
  assert(__nv_fp128_exp2(0.q) == 1.q);
  assert(__nv_fp128_exp10(0.q) == 1.q);
  assert(__nv_fp128_expm1(0.q) == 0.q);
  assert(__nv_fp128_log(1.q) == 0.q);
  assert(__nv_fp128_log2(1.q) == 0.q);
  assert(__nv_fp128_log10(1.q) == 0.q);
  assert(__nv_fp128_log1p(0.q) == 0.q);
  assert(__nv_fp128_pow(1.q, 1.q) == 1.q);
  assert(__nv_fp128_sinh(0.q) == 0.q);
  assert(__nv_fp128_cosh(0.q) == 1.q);
  assert(__nv_fp128_tanh(0.q) == 0.q);
  assert(__nv_fp128_asinh(0.q) == 0.q);
  assert(__nv_fp128_acosh(1.q) == 0.q);
  assert(__nv_fp128_atanh(0.q) == 0.q);
  assert(__nv_fp128_trunc(1.5q) == 1.q);
  assert(__nv_fp128_floor(1.5q) == 1.q);
  assert(__nv_fp128_ceil(1.5q) == 2.q);
  assert(__nv_fp128_round(1.5q) == 2.q);
  assert(__nv_fp128_rint(1.99q) == 2.q);
  assert(__nv_fp128_fabs(-1.q) == 1.q);
  assert(__nv_fp128_copysign(1.q, -1.q) == -1.q);
  assert(__nv_fp128_fmax(1.q, -1.q) == 1.q);
  assert(__nv_fp128_fmin(1.q, -1.q) == -1.q);
  assert(__nv_fp128_fdim(1.q, 1.q) == 0.q);
  assert(__nv_fp128_fmod(1.q, 1.q) == 0.q);
  assert(__nv_fp128_remainder(1.q, 1.q) == 0.q);
  assert(__nv_fp128_frexp(1.q, &dummy_int) == 1.q);
  assert(__nv_fp128_modf(1.q, &dummy_f128) == 1.q);
  assert(__nv_fp128_hypot(3.q, 4.q) == 5.q);
  assert(__nv_fp128_fma(1.q, 1.q, 1.q) == 2.q);
  assert(__nv_fp128_ldexp(1.q, 0) == 1.q);
  assert(__nv_fp128_ilogb(cuda::std::numeric_limits<__float128>::infinity()) == cuda::std::numeric_limits<int>::max());
  assert(__nv_fp128_mul(1.q, 1.q) == 1.q);
  assert(__nv_fp128_add(1.q, 1.q) == 2.q);
  assert(__nv_fp128_sub(1.q, 1.q) == 0.q);
  assert(__nv_fp128_div(1.q, 1.q) == 1.q);
  assert(__nv_fp128_isnan(1.q) == false);
  assert(__nv_fp128_isunordered(1.q, 1.q) == false);
#endif // _CCCL_HAS_FLOAT128() && _CCCL_DEVICE_COMPILATION() && _CCCL_CTK_AT_LEAST(12, 8)
}

int main(int, char**)
{
  NV_IF_TARGET(NV_IS_DEVICE, (test();))
  return 0;
}
