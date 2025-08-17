//===-- language/Compability/Runtime/CUDA/descriptor.h -----------------*- C++ -*-===//
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

#ifndef LANGUAGE_COMPABILITY_RUNTIME_CUDA_DESCRIPTOR_H_
#define LANGUAGE_COMPABILITY_RUNTIME_CUDA_DESCRIPTOR_H_

#include "language/Compability/Runtime/descriptor-consts.h"
#include "language/Compability/Runtime/entry-names.h"
#include <cstddef>

namespace language::Compability::runtime::cuda {

extern "C" {

/// Allocate a descriptor in managed.
Descriptor *RTDECL(CUFAllocDescriptor)(
    std::size_t, const char *sourceFile = nullptr, int sourceLine = 0);

/// Deallocate a descriptor allocated in managed or unified memory.
void RTDECL(CUFFreeDescriptor)(
    Descriptor *, const char *sourceFile = nullptr, int sourceLine = 0);

/// Retrieve the device pointer from the host one.
void *RTDECL(CUFGetDeviceAddress)(
    void *hostPtr, const char *sourceFile = nullptr, int sourceLine = 0);

/// Sync the \p src descriptor to the \p dst descriptor.
void RTDECL(CUFDescriptorSync)(Descriptor *dst, const Descriptor *src,
    const char *sourceFile = nullptr, int sourceLine = 0);

/// Get the device address of registered with the \p hostPtr and sync them.
void RTDECL(CUFSyncGlobalDescriptor)(
    void *hostPtr, const char *sourceFile = nullptr, int sourceLine = 0);

/// Check descriptor passed to a kernel.
void RTDECL(CUFDescriptorCheckSection)(
    const Descriptor *, const char *sourceFile = nullptr, int sourceLine = 0);

/// Set the allocator index with the provided value.
void RTDECL(CUFSetAllocatorIndex)(Descriptor *, int index,
    const char *sourceFile = nullptr, int sourceLine = 0);

} // extern "C"

} // namespace language::Compability::runtime::cuda
#endif // FORTRAN_RUNTIME_CUDA_DESCRIPTOR_H_
