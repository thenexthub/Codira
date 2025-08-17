//===-- language/Compability/Runtime/CUDA/registration.h ---------------*- C -*-===//
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

#ifndef LANGUAGE_COMPABILITY_RUNTIME_CUDA_REGISTRATION_H_
#define LANGUAGE_COMPABILITY_RUNTIME_CUDA_REGISTRATION_H_

#include "language/Compability/Runtime/entry-names.h"
#include <cstddef>
#include <cstdint>

namespace language::Compability::runtime::cuda {

extern "C" {

/// Register a CUDA module.
void *RTDECL(CUFRegisterModule)(void *data);

/// Register a device function.
void RTDECL(CUFRegisterFunction)(
    void **module, const char *fctSym, char *fctName);

/// Register a device variable.
void RTDECL(CUFRegisterVariable)(
    void **module, char *varSym, const char *varName, int64_t size);

} // extern "C"

} // namespace language::Compability::runtime::cuda
#endif // FORTRAN_RUNTIME_CUDA_REGISTRATION_H_
