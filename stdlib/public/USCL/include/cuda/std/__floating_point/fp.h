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

#ifndef _CUDA_STD___FLOATING_POINT_FP_H
#define _CUDA_STD___FLOATING_POINT_FP_H

#include <uscl/__cccl_config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/std/__floating_point/arithmetic.h>
#include <uscl/std/__floating_point/cast.h>
#include <uscl/std/__floating_point/cccl_fp.h>
#include <uscl/std/__floating_point/common_type.h>
#include <uscl/std/__floating_point/constants.h>
#include <uscl/std/__floating_point/conversion_rank_order.h>
#include <uscl/std/__floating_point/cuda_fp_types.h>
#include <uscl/std/__floating_point/format.h>
#include <uscl/std/__floating_point/mask.h>
#include <uscl/std/__floating_point/native_type.h>
#include <uscl/std/__floating_point/overflow_handler.h>
#include <uscl/std/__floating_point/properties.h>
#include <uscl/std/__floating_point/storage.h>
#include <uscl/std/__floating_point/traits.h>

#endif // _CUDA_STD___FLOATING_POINT_FP_H
