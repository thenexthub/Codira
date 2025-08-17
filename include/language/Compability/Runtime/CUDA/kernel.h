//===-- language/Compability/Runtime/CUDA/kernel.h ---------------------*- C++ -*-===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_COMPABILITY_RUNTIME_CUDA_KERNEL_H_
#define LANGUAGE_COMPABILITY_RUNTIME_CUDA_KERNEL_H_

#include "language/Compability/Runtime/entry-names.h"
#include <cstddef>
#include <stdint.h>

extern "C" {

// These functions use intptr_t instead of CUDA's unsigned int to match
// the type of MLIR's index type. This avoids the need for casts in the
// generated MLIR code.

void RTDEF(CUFLaunchKernel)(const void *kernelName, intptr_t gridX,
    intptr_t gridY, intptr_t gridZ, intptr_t blockX, intptr_t blockY,
    intptr_t blockZ, int64_t *stream, int32_t smem, void **params,
    void **extra);

void RTDEF(CUFLaunchClusterKernel)(const void *kernelName, intptr_t clusterX,
    intptr_t clusterY, intptr_t clusterZ, intptr_t gridX, intptr_t gridY,
    intptr_t gridZ, intptr_t blockX, intptr_t blockY, intptr_t blockZ,
    int64_t *stream, int32_t smem, void **params, void **extra);

void RTDEF(CUFLaunchCooperativeKernel)(const void *kernelName, intptr_t gridX,
    intptr_t gridY, intptr_t gridZ, intptr_t blockX, intptr_t blockY,
    intptr_t blockZ, int64_t *stream, int32_t smem, void **params,
    void **extra);

} // extern "C"

#endif // FORTRAN_RUNTIME_CUDA_KERNEL_H_
