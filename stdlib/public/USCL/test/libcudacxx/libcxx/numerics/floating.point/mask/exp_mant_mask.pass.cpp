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

#include <uscl/std/__floating_point/fp.h>
#include <uscl/std/cassert>
#include <uscl/std/type_traits>

#include "test_macros.h"

template <cuda::std::__fp_format Fmt>
__host__ __device__ constexpr bool test_fp_exp_mant_mask(cuda::std::__fp_storage_t<Fmt> expected)
{
  using T = cuda::std::__cccl_fp<Fmt>;

  static_assert(cuda::std::is_same_v<cuda::std::remove_cv_t<decltype(cuda::std::__fp_exp_mant_mask_v<Fmt>)>,
                                     cuda::std::__fp_storage_t<Fmt>>);
  static_assert(cuda::std::is_same_v<cuda::std::remove_cv_t<decltype(cuda::std::__fp_exp_mant_mask_of_v<T>)>,
                                     cuda::std::__fp_storage_t<Fmt>>);

  assert(cuda::std::__fp_exp_mant_mask_v<Fmt> == expected);
  assert(cuda::std::__fp_exp_mant_mask_of_v<T> == expected);

  return true;
}

static_assert(test_fp_exp_mant_mask<cuda::std::__fp_format::__binary16>(0x7fffu));
static_assert(test_fp_exp_mant_mask<cuda::std::__fp_format::__binary32>(0x7fffffffu));
static_assert(test_fp_exp_mant_mask<cuda::std::__fp_format::__binary64>(0x7fffffffffffffffull));
#if _CCCL_HAS_INT128()
static_assert(test_fp_exp_mant_mask<cuda::std::__fp_format::__binary128>(
  (__uint128_t(0x7fffffffffffffffull) << 64) | 0xffffffffffffffffull));
#endif // _CCCL_HAS_INT128()
static_assert(test_fp_exp_mant_mask<cuda::std::__fp_format::__bfloat16>(0x7fffu));
#if _CCCL_HAS_INT128()
static_assert(
  test_fp_exp_mant_mask<cuda::std::__fp_format::__fp80_x86>((__uint128_t(0x7fffull) << 64) | 0xffffffffffffffffull));
#endif // _CCCL_HAS_INT128()
static_assert(test_fp_exp_mant_mask<cuda::std::__fp_format::__fp8_nv_e4m3>(0x7fu));
static_assert(test_fp_exp_mant_mask<cuda::std::__fp_format::__fp8_nv_e5m2>(0x7fu));
static_assert(test_fp_exp_mant_mask<cuda::std::__fp_format::__fp8_nv_e8m0>(0xffu));
static_assert(test_fp_exp_mant_mask<cuda::std::__fp_format::__fp6_nv_e2m3>(0x1fu));
static_assert(test_fp_exp_mant_mask<cuda::std::__fp_format::__fp6_nv_e3m2>(0x1fu));
static_assert(test_fp_exp_mant_mask<cuda::std::__fp_format::__fp4_nv_e2m1>(0x7u));

int main(int, char**)
{
  return 0;
}
