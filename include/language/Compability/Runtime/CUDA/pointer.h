//===-- language/Compability/Runtime/CUDA/pointer.h --------------------*- C++ -*-===//
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

#ifndef LANGUAGE_COMPABILITY_RUNTIME_CUDA_POINTER_H_
#define LANGUAGE_COMPABILITY_RUNTIME_CUDA_POINTER_H_

#include "language/Compability/Runtime/descriptor-consts.h"
#include "language/Compability/Runtime/entry-names.h"

namespace language::Compability::runtime::cuda {

extern "C" {

/// Perform allocation of the descriptor.
int RTDECL(CUFPointerAllocate)(Descriptor &, int64_t *stream = nullptr,
    bool *pinned = nullptr, bool hasStat = false,
    const Descriptor *errMsg = nullptr, const char *sourceFile = nullptr,
    int sourceLine = 0);

/// Perform allocation of the descriptor with synchronization of it when
/// necessary.
int RTDECL(CUFPointerAllocateSync)(Descriptor &, int64_t *stream = nullptr,
    bool *pinned = nullptr, bool hasStat = false,
    const Descriptor *errMsg = nullptr, const char *sourceFile = nullptr,
    int sourceLine = 0);

/// Perform allocation of the descriptor without synchronization. Assign data
/// from source.
int RTDEF(CUFPointerAllocateSource)(Descriptor &pointer,
    const Descriptor &source, int64_t *stream = nullptr, bool *pinned = nullptr,
    bool hasStat = false, const Descriptor *errMsg = nullptr,
    const char *sourceFile = nullptr, int sourceLine = 0);

/// Perform allocation of the descriptor with synchronization of it when
/// necessary. Assign data from source.
int RTDEF(CUFPointerAllocateSourceSync)(Descriptor &pointer,
    const Descriptor &source, int64_t *stream = nullptr, bool *pinned = nullptr,
    bool hasStat = false, const Descriptor *errMsg = nullptr,
    const char *sourceFile = nullptr, int sourceLine = 0);

} // extern "C"

} // namespace language::Compability::runtime::cuda
#endif // FORTRAN_RUNTIME_CUDA_POINTER_H_
