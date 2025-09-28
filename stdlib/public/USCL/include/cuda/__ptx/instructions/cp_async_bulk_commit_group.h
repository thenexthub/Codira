/*
 *
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 * Date: Friday, December 29, 2023.
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

#ifndef _CUDA_PTX_CP_ASYNC_BULK_COMMIT_GROUP_H_
#define _CUDA_PTX_CP_ASYNC_BULK_COMMIT_GROUP_H_

#include <uscl/std/detail/__config>

#if defined(_CCCL_IMPLICIT_SYSTEM_HEADER_GCC)
#  pragma GCC system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_CLANG)
#  pragma clang system_header
#elif defined(_CCCL_IMPLICIT_SYSTEM_HEADER_MSVC)
#  pragma system_header
#endif // no system header

#include <uscl/__ptx/ptx_dot_variants.h>
#include <uscl/__ptx/ptx_helper_functions.h>
#include <uscl/std/cstdint>

#include <nv/target> // __CUDA_MINIMUM_ARCH__ and friends

#include <uscl/std/__cccl/prologue.h>

_CCCL_BEGIN_NAMESPACE_CUDA_PTX

// 9.7.8.24.12. Data Movement and Conversion Instructions: cp.async.bulk.commit_group
// https://docs.nvidia.com/cuda/parallel-thread-execution/index.html#data-movement-and-conversion-instructions-cp-async-bulk-commit-group
#include <uscl/__ptx/instructions/generated/cp_async_bulk_commit_group.h>

_CCCL_END_NAMESPACE_CUDA_PTX

#include <uscl/std/__cccl/epilogue.h>

#endif // _CUDA_PTX_CP_ASYNC_BULK_COMMIT_GROUP_H_
