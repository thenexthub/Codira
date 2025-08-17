//===-- language/Compability/Runtime/inquiry.h ----------------*- C++ -*-===//
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

// Defines the API for the inquiry intrinsic functions
// that inquire about shape information in arrays: LBOUND and SIZE.

#ifndef LANGUAGE_COMPABILITY_RUNTIME_INQUIRY_H_
#define LANGUAGE_COMPABILITY_RUNTIME_INQUIRY_H_

#include "language/Compability/Runtime/entry-names.h"
#include <cinttypes>

namespace language::Compability::runtime {

class Descriptor;

extern "C" {

std::int64_t RTDECL(LboundDim)(const Descriptor &array, int dim,
    const char *sourceFile = nullptr, int line = 0);

void RTDECL(Lbound)(void *result, const Descriptor &array, int kind,
    const char *sourceFile = nullptr, int line = 0);

void RTDECL(Shape)(void *result, const Descriptor &array, int kind,
    const char *sourceFile = nullptr, int line = 0);
std::int64_t RTDECL(Size)(
    const Descriptor &array, const char *sourceFile = nullptr, int line = 0);

std::int64_t RTDECL(SizeDim)(const Descriptor &array, int dim,
    const char *sourceFile = nullptr, int line = 0);

void RTDECL(Ubound)(void *result, const Descriptor &array, int kind,
    const char *sourceFile = nullptr, int line = 0);

} // extern "C"
} // namespace language::Compability::runtime
#endif // FORTRAN_RUNTIME_INQUIRY_H_
