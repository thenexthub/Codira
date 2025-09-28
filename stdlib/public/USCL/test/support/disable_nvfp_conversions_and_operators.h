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

#ifndef SUPPORT_DISABLE_NVFP_CONVERSIONS_AND_OPERATORS_H
#define SUPPORT_DISABLE_NVFP_CONVERSIONS_AND_OPERATORS_H

#define __CUDA_NO_FP4_CONVERSIONS__          1
#define __CUDA_NO_FP4_CONVERSION_OPERATORS__ 1
#define __CUDA_NO_FP6_CONVERSIONS__          1
#define __CUDA_NO_FP6_CONVERSION_OPERATORS__ 1
#define __CUDA_NO_FP8_CONVERSIONS__          1
#define __CUDA_NO_FP8_CONVERSION_OPERATORS__ 1
#define __CUDA_NO_HALF_CONVERSIONS__         1
#define __CUDA_NO_HALF_OPERATORS__           1
#define __CUDA_NO_HALF2_OPERATORS__          1
#define __CUDA_NO_BFLOAT16_CONVERSIONS__     1
#define __CUDA_NO_BFLOAT16_OPERATORS__       1
#define __CUDA_NO_BFLOAT162_OPERATORS__      1

#endif // SUPPORT_DISABLE_NVFP_CONVERSIONS_AND_OPERATORS_H
