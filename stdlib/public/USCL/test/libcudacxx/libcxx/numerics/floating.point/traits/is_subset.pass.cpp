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

using cuda::std::__fp_format;
using cuda::std::__fp_is_subset_v;

static_assert(__fp_is_subset_v<__fp_format::__binary16, __fp_format::__binary16>);
static_assert(__fp_is_subset_v<__fp_format::__binary16, __fp_format::__binary32>);
static_assert(__fp_is_subset_v<__fp_format::__binary16, __fp_format::__binary64>);
static_assert(__fp_is_subset_v<__fp_format::__binary16, __fp_format::__binary128>);
static_assert(!__fp_is_subset_v<__fp_format::__binary16, __fp_format::__bfloat16>);
static_assert(__fp_is_subset_v<__fp_format::__binary16, __fp_format::__fp80_x86>);
static_assert(!__fp_is_subset_v<__fp_format::__binary16, __fp_format::__fp8_nv_e4m3>);
static_assert(!__fp_is_subset_v<__fp_format::__binary16, __fp_format::__fp8_nv_e5m2>);
static_assert(!__fp_is_subset_v<__fp_format::__binary16, __fp_format::__fp8_nv_e8m0>);
static_assert(!__fp_is_subset_v<__fp_format::__binary16, __fp_format::__fp6_nv_e2m3>);
static_assert(!__fp_is_subset_v<__fp_format::__binary16, __fp_format::__fp6_nv_e3m2>);
static_assert(!__fp_is_subset_v<__fp_format::__binary16, __fp_format::__fp4_nv_e2m1>);

static_assert(!__fp_is_subset_v<__fp_format::__binary32, __fp_format::__binary16>);
static_assert(__fp_is_subset_v<__fp_format::__binary32, __fp_format::__binary32>);
static_assert(__fp_is_subset_v<__fp_format::__binary32, __fp_format::__binary64>);
static_assert(__fp_is_subset_v<__fp_format::__binary32, __fp_format::__binary128>);
static_assert(!__fp_is_subset_v<__fp_format::__binary32, __fp_format::__bfloat16>);
static_assert(__fp_is_subset_v<__fp_format::__binary32, __fp_format::__fp80_x86>);
static_assert(!__fp_is_subset_v<__fp_format::__binary32, __fp_format::__fp8_nv_e4m3>);
static_assert(!__fp_is_subset_v<__fp_format::__binary32, __fp_format::__fp8_nv_e5m2>);
static_assert(!__fp_is_subset_v<__fp_format::__binary32, __fp_format::__fp8_nv_e8m0>);
static_assert(!__fp_is_subset_v<__fp_format::__binary32, __fp_format::__fp6_nv_e2m3>);
static_assert(!__fp_is_subset_v<__fp_format::__binary32, __fp_format::__fp6_nv_e3m2>);
static_assert(!__fp_is_subset_v<__fp_format::__binary32, __fp_format::__fp4_nv_e2m1>);

static_assert(!__fp_is_subset_v<__fp_format::__binary64, __fp_format::__binary16>);
static_assert(!__fp_is_subset_v<__fp_format::__binary64, __fp_format::__binary32>);
static_assert(__fp_is_subset_v<__fp_format::__binary64, __fp_format::__binary64>);
static_assert(__fp_is_subset_v<__fp_format::__binary64, __fp_format::__binary128>);
static_assert(!__fp_is_subset_v<__fp_format::__binary64, __fp_format::__bfloat16>);
static_assert(__fp_is_subset_v<__fp_format::__binary64, __fp_format::__fp80_x86>);
static_assert(!__fp_is_subset_v<__fp_format::__binary64, __fp_format::__fp8_nv_e4m3>);
static_assert(!__fp_is_subset_v<__fp_format::__binary64, __fp_format::__fp8_nv_e5m2>);
static_assert(!__fp_is_subset_v<__fp_format::__binary64, __fp_format::__fp8_nv_e8m0>);
static_assert(!__fp_is_subset_v<__fp_format::__binary64, __fp_format::__fp6_nv_e2m3>);
static_assert(!__fp_is_subset_v<__fp_format::__binary64, __fp_format::__fp6_nv_e3m2>);
static_assert(!__fp_is_subset_v<__fp_format::__binary64, __fp_format::__fp4_nv_e2m1>);

static_assert(!__fp_is_subset_v<__fp_format::__binary128, __fp_format::__binary16>);
static_assert(!__fp_is_subset_v<__fp_format::__binary128, __fp_format::__binary32>);
static_assert(!__fp_is_subset_v<__fp_format::__binary128, __fp_format::__binary64>);
static_assert(__fp_is_subset_v<__fp_format::__binary128, __fp_format::__binary128>);
static_assert(!__fp_is_subset_v<__fp_format::__binary128, __fp_format::__bfloat16>);
static_assert(!__fp_is_subset_v<__fp_format::__binary128, __fp_format::__fp80_x86>);
static_assert(!__fp_is_subset_v<__fp_format::__binary128, __fp_format::__fp8_nv_e4m3>);
static_assert(!__fp_is_subset_v<__fp_format::__binary128, __fp_format::__fp8_nv_e5m2>);
static_assert(!__fp_is_subset_v<__fp_format::__binary128, __fp_format::__fp8_nv_e8m0>);
static_assert(!__fp_is_subset_v<__fp_format::__binary128, __fp_format::__fp6_nv_e2m3>);
static_assert(!__fp_is_subset_v<__fp_format::__binary128, __fp_format::__fp6_nv_e3m2>);
static_assert(!__fp_is_subset_v<__fp_format::__binary128, __fp_format::__fp4_nv_e2m1>);

static_assert(!__fp_is_subset_v<__fp_format::__bfloat16, __fp_format::__binary16>);
static_assert(__fp_is_subset_v<__fp_format::__bfloat16, __fp_format::__binary32>);
static_assert(__fp_is_subset_v<__fp_format::__bfloat16, __fp_format::__binary64>);
static_assert(__fp_is_subset_v<__fp_format::__bfloat16, __fp_format::__binary128>);
static_assert(__fp_is_subset_v<__fp_format::__bfloat16, __fp_format::__bfloat16>);
static_assert(__fp_is_subset_v<__fp_format::__bfloat16, __fp_format::__fp80_x86>);
static_assert(!__fp_is_subset_v<__fp_format::__bfloat16, __fp_format::__fp8_nv_e4m3>);
static_assert(!__fp_is_subset_v<__fp_format::__bfloat16, __fp_format::__fp8_nv_e5m2>);
static_assert(!__fp_is_subset_v<__fp_format::__bfloat16, __fp_format::__fp8_nv_e8m0>);
static_assert(!__fp_is_subset_v<__fp_format::__bfloat16, __fp_format::__fp6_nv_e2m3>);
static_assert(!__fp_is_subset_v<__fp_format::__bfloat16, __fp_format::__fp6_nv_e3m2>);
static_assert(!__fp_is_subset_v<__fp_format::__bfloat16, __fp_format::__fp4_nv_e2m1>);

static_assert(!__fp_is_subset_v<__fp_format::__fp80_x86, __fp_format::__binary16>);
static_assert(!__fp_is_subset_v<__fp_format::__fp80_x86, __fp_format::__binary32>);
static_assert(!__fp_is_subset_v<__fp_format::__fp80_x86, __fp_format::__binary64>);
static_assert(__fp_is_subset_v<__fp_format::__fp80_x86, __fp_format::__binary128>);
static_assert(!__fp_is_subset_v<__fp_format::__fp80_x86, __fp_format::__bfloat16>);
static_assert(__fp_is_subset_v<__fp_format::__fp80_x86, __fp_format::__fp80_x86>);
static_assert(!__fp_is_subset_v<__fp_format::__fp80_x86, __fp_format::__fp8_nv_e4m3>);
static_assert(!__fp_is_subset_v<__fp_format::__fp80_x86, __fp_format::__fp8_nv_e5m2>);
static_assert(!__fp_is_subset_v<__fp_format::__fp80_x86, __fp_format::__fp8_nv_e8m0>);
static_assert(!__fp_is_subset_v<__fp_format::__fp80_x86, __fp_format::__fp6_nv_e2m3>);
static_assert(!__fp_is_subset_v<__fp_format::__fp80_x86, __fp_format::__fp6_nv_e3m2>);
static_assert(!__fp_is_subset_v<__fp_format::__fp80_x86, __fp_format::__fp4_nv_e2m1>);

static_assert(__fp_is_subset_v<__fp_format::__fp8_nv_e4m3, __fp_format::__binary16>);
static_assert(__fp_is_subset_v<__fp_format::__fp8_nv_e4m3, __fp_format::__binary32>);
static_assert(__fp_is_subset_v<__fp_format::__fp8_nv_e4m3, __fp_format::__binary64>);
static_assert(__fp_is_subset_v<__fp_format::__fp8_nv_e4m3, __fp_format::__binary128>);
static_assert(__fp_is_subset_v<__fp_format::__fp8_nv_e4m3, __fp_format::__bfloat16>);
static_assert(__fp_is_subset_v<__fp_format::__fp8_nv_e4m3, __fp_format::__fp80_x86>);
static_assert(__fp_is_subset_v<__fp_format::__fp8_nv_e4m3, __fp_format::__fp8_nv_e4m3>);
static_assert(!__fp_is_subset_v<__fp_format::__fp8_nv_e4m3, __fp_format::__fp8_nv_e5m2>);
static_assert(!__fp_is_subset_v<__fp_format::__fp8_nv_e4m3, __fp_format::__fp8_nv_e8m0>);
static_assert(!__fp_is_subset_v<__fp_format::__fp8_nv_e4m3, __fp_format::__fp6_nv_e2m3>);
static_assert(!__fp_is_subset_v<__fp_format::__fp8_nv_e4m3, __fp_format::__fp6_nv_e3m2>);
static_assert(!__fp_is_subset_v<__fp_format::__fp8_nv_e4m3, __fp_format::__fp4_nv_e2m1>);

static_assert(__fp_is_subset_v<__fp_format::__fp8_nv_e5m2, __fp_format::__binary16>);
static_assert(__fp_is_subset_v<__fp_format::__fp8_nv_e5m2, __fp_format::__binary32>);
static_assert(__fp_is_subset_v<__fp_format::__fp8_nv_e5m2, __fp_format::__binary64>);
static_assert(__fp_is_subset_v<__fp_format::__fp8_nv_e5m2, __fp_format::__binary128>);
static_assert(__fp_is_subset_v<__fp_format::__fp8_nv_e5m2, __fp_format::__bfloat16>);
static_assert(__fp_is_subset_v<__fp_format::__fp8_nv_e5m2, __fp_format::__fp80_x86>);
static_assert(!__fp_is_subset_v<__fp_format::__fp8_nv_e5m2, __fp_format::__fp8_nv_e4m3>);
static_assert(__fp_is_subset_v<__fp_format::__fp8_nv_e5m2, __fp_format::__fp8_nv_e5m2>);
static_assert(!__fp_is_subset_v<__fp_format::__fp8_nv_e5m2, __fp_format::__fp8_nv_e8m0>);
static_assert(!__fp_is_subset_v<__fp_format::__fp8_nv_e5m2, __fp_format::__fp6_nv_e2m3>);
static_assert(!__fp_is_subset_v<__fp_format::__fp8_nv_e5m2, __fp_format::__fp6_nv_e3m2>);
static_assert(!__fp_is_subset_v<__fp_format::__fp8_nv_e5m2, __fp_format::__fp4_nv_e2m1>);

static_assert(!__fp_is_subset_v<__fp_format::__fp8_nv_e8m0, __fp_format::__binary16>);
static_assert(!__fp_is_subset_v<__fp_format::__fp8_nv_e8m0, __fp_format::__binary32>);
static_assert(__fp_is_subset_v<__fp_format::__fp8_nv_e8m0, __fp_format::__binary64>);
static_assert(__fp_is_subset_v<__fp_format::__fp8_nv_e8m0, __fp_format::__binary128>);
static_assert(!__fp_is_subset_v<__fp_format::__fp8_nv_e8m0, __fp_format::__bfloat16>);
static_assert(__fp_is_subset_v<__fp_format::__fp8_nv_e8m0, __fp_format::__fp80_x86>);
static_assert(!__fp_is_subset_v<__fp_format::__fp8_nv_e8m0, __fp_format::__fp8_nv_e4m3>);
static_assert(!__fp_is_subset_v<__fp_format::__fp8_nv_e8m0, __fp_format::__fp8_nv_e5m2>);
static_assert(__fp_is_subset_v<__fp_format::__fp8_nv_e8m0, __fp_format::__fp8_nv_e8m0>);
static_assert(!__fp_is_subset_v<__fp_format::__fp8_nv_e8m0, __fp_format::__fp6_nv_e2m3>);
static_assert(!__fp_is_subset_v<__fp_format::__fp8_nv_e8m0, __fp_format::__fp6_nv_e3m2>);
static_assert(!__fp_is_subset_v<__fp_format::__fp8_nv_e8m0, __fp_format::__fp4_nv_e2m1>);

static_assert(__fp_is_subset_v<__fp_format::__fp6_nv_e2m3, __fp_format::__binary16>);
static_assert(__fp_is_subset_v<__fp_format::__fp6_nv_e2m3, __fp_format::__binary32>);
static_assert(__fp_is_subset_v<__fp_format::__fp6_nv_e2m3, __fp_format::__binary64>);
static_assert(__fp_is_subset_v<__fp_format::__fp6_nv_e2m3, __fp_format::__binary128>);
static_assert(__fp_is_subset_v<__fp_format::__fp6_nv_e2m3, __fp_format::__bfloat16>);
static_assert(__fp_is_subset_v<__fp_format::__fp6_nv_e2m3, __fp_format::__fp80_x86>);
static_assert(__fp_is_subset_v<__fp_format::__fp6_nv_e2m3, __fp_format::__fp8_nv_e4m3>);
static_assert(!__fp_is_subset_v<__fp_format::__fp6_nv_e2m3, __fp_format::__fp8_nv_e5m2>);
static_assert(!__fp_is_subset_v<__fp_format::__fp6_nv_e2m3, __fp_format::__fp8_nv_e8m0>);
static_assert(__fp_is_subset_v<__fp_format::__fp6_nv_e2m3, __fp_format::__fp6_nv_e2m3>);
static_assert(!__fp_is_subset_v<__fp_format::__fp6_nv_e2m3, __fp_format::__fp6_nv_e3m2>);
static_assert(!__fp_is_subset_v<__fp_format::__fp6_nv_e2m3, __fp_format::__fp4_nv_e2m1>);

static_assert(__fp_is_subset_v<__fp_format::__fp6_nv_e3m2, __fp_format::__binary16>);
static_assert(__fp_is_subset_v<__fp_format::__fp6_nv_e3m2, __fp_format::__binary32>);
static_assert(__fp_is_subset_v<__fp_format::__fp6_nv_e3m2, __fp_format::__binary64>);
static_assert(__fp_is_subset_v<__fp_format::__fp6_nv_e3m2, __fp_format::__binary128>);
static_assert(__fp_is_subset_v<__fp_format::__fp6_nv_e3m2, __fp_format::__bfloat16>);
static_assert(__fp_is_subset_v<__fp_format::__fp6_nv_e3m2, __fp_format::__fp80_x86>);
static_assert(__fp_is_subset_v<__fp_format::__fp6_nv_e3m2, __fp_format::__fp8_nv_e4m3>);
static_assert(__fp_is_subset_v<__fp_format::__fp6_nv_e3m2, __fp_format::__fp8_nv_e5m2>);
static_assert(!__fp_is_subset_v<__fp_format::__fp6_nv_e3m2, __fp_format::__fp8_nv_e8m0>);
static_assert(!__fp_is_subset_v<__fp_format::__fp6_nv_e3m2, __fp_format::__fp6_nv_e2m3>);
static_assert(__fp_is_subset_v<__fp_format::__fp6_nv_e3m2, __fp_format::__fp6_nv_e3m2>);
static_assert(!__fp_is_subset_v<__fp_format::__fp6_nv_e3m2, __fp_format::__fp4_nv_e2m1>);

static_assert(__fp_is_subset_v<__fp_format::__fp4_nv_e2m1, __fp_format::__binary16>);
static_assert(__fp_is_subset_v<__fp_format::__fp4_nv_e2m1, __fp_format::__binary32>);
static_assert(__fp_is_subset_v<__fp_format::__fp4_nv_e2m1, __fp_format::__binary64>);
static_assert(__fp_is_subset_v<__fp_format::__fp4_nv_e2m1, __fp_format::__binary128>);
static_assert(__fp_is_subset_v<__fp_format::__fp4_nv_e2m1, __fp_format::__bfloat16>);
static_assert(__fp_is_subset_v<__fp_format::__fp4_nv_e2m1, __fp_format::__fp80_x86>);
static_assert(__fp_is_subset_v<__fp_format::__fp4_nv_e2m1, __fp_format::__fp8_nv_e4m3>);
static_assert(__fp_is_subset_v<__fp_format::__fp4_nv_e2m1, __fp_format::__fp8_nv_e5m2>);
static_assert(!__fp_is_subset_v<__fp_format::__fp4_nv_e2m1, __fp_format::__fp8_nv_e8m0>);
static_assert(__fp_is_subset_v<__fp_format::__fp4_nv_e2m1, __fp_format::__fp6_nv_e2m3>);
static_assert(__fp_is_subset_v<__fp_format::__fp4_nv_e2m1, __fp_format::__fp6_nv_e3m2>);
static_assert(__fp_is_subset_v<__fp_format::__fp4_nv_e2m1, __fp_format::__fp4_nv_e2m1>);

int main(int, char**)
{
  return 0;
}
