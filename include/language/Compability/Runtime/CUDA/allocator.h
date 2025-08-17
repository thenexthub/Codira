//===-- language/Compability/Runtime/CUDA/allocator.h ------------------*- C++ -*-===//
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

#ifndef LANGUAGE_COMPABILITY_RUNTIME_CUDA_ALLOCATOR_H_
#define LANGUAGE_COMPABILITY_RUNTIME_CUDA_ALLOCATOR_H_

#include "common.h"
#include "language/Compability/Runtime/descriptor-consts.h"
#include "language/Compability/Runtime/entry-names.h"

namespace language::Compability::runtime::cuda {

extern "C" {

void RTDECL(CUFRegisterAllocator)();
}

void *CUFAllocPinned(std::size_t, std::int64_t *);
void CUFFreePinned(void *);

void *CUFAllocDevice(std::size_t, std::int64_t *);
void CUFFreeDevice(void *);

void *CUFAllocManaged(std::size_t, std::int64_t *);
void CUFFreeManaged(void *);

void *CUFAllocUnified(std::size_t, std::int64_t *);
void CUFFreeUnified(void *);

} // namespace language::Compability::runtime::cuda
#endif // FORTRAN_RUNTIME_CUDA_ALLOCATOR_H_
