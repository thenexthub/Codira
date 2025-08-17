//===-- language/Compability/Runtime/CUDA/common.h ------------------*- C++ -*-===//
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

#ifndef LANGUAGE_COMPABILITY_RUNTIME_CUDA_COMMON_H_
#define LANGUAGE_COMPABILITY_RUNTIME_CUDA_COMMON_H_

#include "language/Compability/Runtime/descriptor-consts.h"
#include "language/Compability/Runtime/entry-names.h"

/// Type of memory for allocation/deallocation
static constexpr unsigned kMemTypeDevice = 0;
static constexpr unsigned kMemTypeManaged = 1;
static constexpr unsigned kMemTypeUnified = 2;
static constexpr unsigned kMemTypePinned = 3;

/// Data transfer kinds.
static constexpr unsigned kHostToDevice = 0;
static constexpr unsigned kDeviceToHost = 1;
static constexpr unsigned kDeviceToDevice = 2;

#define CUDA_REPORT_IF_ERROR(expr) \
  [](cudaError_t err) { \
    if (err == cudaSuccess) \
      return; \
    const char *name = cudaGetErrorName(err); \
    if (!name) \
      name = "<unknown>"; \
    language::Compability::runtime::Terminator terminator{__FILE__, __LINE__}; \
    terminator.Crash("'%s' failed with '%s'", #expr, name); \
  }(expr)

#endif // FORTRAN_RUNTIME_CUDA_COMMON_H_
