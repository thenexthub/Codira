//===- SpirvCpuRuntimeWrappers.cpp - Runner testing library -===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
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
//
// A small library for SPIR-V cpu runner testing.
//
//===----------------------------------------------------------------------===//

#include "mlir/ExecutionEngine/CRunnerUtils.h"

#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __attribute__((visibility("default")))
#endif

// NOLINTBEGIN(*-identifier-naming)

extern "C" EXPORT void
_mlir_ciface_fillI32Buffer(StridedMemRefType<int32_t, 1> *mem_ref,
                           int32_t value) {
  std::fill_n(mem_ref->basePtr, mem_ref->sizes[0], value);
}

extern "C" EXPORT void
_mlir_ciface_fillF32Buffer1D(StridedMemRefType<float, 1> *mem_ref,
                             float value) {
  std::fill_n(mem_ref->basePtr, mem_ref->sizes[0], value);
}

extern "C" EXPORT void
_mlir_ciface_fillF32Buffer2D(StridedMemRefType<float, 2> *mem_ref,
                             float value) {
  std::fill_n(mem_ref->basePtr, mem_ref->sizes[0] * mem_ref->sizes[1], value);
}

extern "C" EXPORT void
_mlir_ciface_fillF32Buffer3D(StridedMemRefType<float, 3> *mem_ref,
                             float value) {
  std::fill_n(mem_ref->basePtr,
              mem_ref->sizes[0] * mem_ref->sizes[1] * mem_ref->sizes[2], value);
}

// NOLINTEND(*-identifier-naming)
