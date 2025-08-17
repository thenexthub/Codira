//===-- language/Compability/Runtime/CUDA/memory.h ---------------------*- C++ -*-===//
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

#ifndef LANGUAGE_COMPABILITY_RUNTIME_CUDA_MEMORY_H_
#define LANGUAGE_COMPABILITY_RUNTIME_CUDA_MEMORY_H_

#include "language/Compability/Runtime/descriptor-consts.h"
#include "language/Compability/Runtime/entry-names.h"
#include <cstddef>

namespace language::Compability::runtime::cuda {

extern "C" {

/// Allocate memory on the device.
void *RTDECL(CUFMemAlloc)(std::size_t bytes, unsigned type,
    const char *sourceFile = nullptr, int sourceLine = 0);

/// Free memory allocated on the device.
void RTDECL(CUFMemFree)(void *devicePtr, unsigned type,
    const char *sourceFile = nullptr, int sourceLine = 0);

/// Set value to the data hold by a descriptor. The \p value pointer must be
/// addressable to the same amount of bytes specified by the element size of
/// the descriptor \p desc.
void RTDECL(CUFMemsetDescriptor)(Descriptor *desc, void *value,
    const char *sourceFile = nullptr, int sourceLine = 0);

/// Data transfer from a pointer to a pointer.
void RTDECL(CUFDataTransferPtrPtr)(void *dst, void *src, std::size_t bytes,
    unsigned mode, const char *sourceFile = nullptr, int sourceLine = 0);

/// Data transfer from a descriptor to a pointer.
void RTDECL(CUFDataTransferPtrDesc)(void *dst, Descriptor *src,
    std::size_t bytes, unsigned mode, const char *sourceFile = nullptr,
    int sourceLine = 0);

/// Data transfer from a descriptor to a descriptor.
void RTDECL(CUFDataTransferDescDesc)(Descriptor *dst, Descriptor *src,
    unsigned mode, const char *sourceFile = nullptr, int sourceLine = 0);

/// Data transfer from a scalar descriptor to a descriptor.
void RTDECL(CUFDataTransferCstDesc)(Descriptor *dst, Descriptor *src,
    unsigned mode, const char *sourceFile = nullptr, int sourceLine = 0);

/// Data transfer from a descriptor to a descriptor.
void RTDECL(CUFDataTransferDescDescNoRealloc)(Descriptor *dst, Descriptor *src,
    unsigned mode, const char *sourceFile = nullptr, int sourceLine = 0);

/// Data transfer from a descriptor to a global descriptor.
void RTDECL(CUFDataTransferGlobalDescDesc)(Descriptor *dst, Descriptor *src,
    unsigned mode, const char *sourceFile = nullptr, int sourceLine = 0);

} // extern "C"
} // namespace language::Compability::runtime::cuda
#endif // FORTRAN_RUNTIME_CUDA_MEMORY_H_
