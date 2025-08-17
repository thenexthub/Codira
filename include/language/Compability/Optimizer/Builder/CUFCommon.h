//===-- CUFCommon.h -------------------------------------------------------===//
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

#ifndef LANGUAGE_COMPABILITY_OPTIMIZER_TRANSFORMS_CUFCOMMON_H_
#define LANGUAGE_COMPABILITY_OPTIMIZER_TRANSFORMS_CUFCOMMON_H_

#include "language/Compability/Optimizer/Dialect/FIROps.h"
#include "mlir/Dialect/GPU/IR/GPUDialect.h"
#include "mlir/IR/BuiltinOps.h"

static constexpr toolchain::StringRef cudaDeviceModuleName = "cuda_device_mod";
static constexpr toolchain::StringRef cudaSharedMemSuffix = "__shared_mem";

namespace fir {
class FirOpBuilder;
} // namespace fir

namespace cuf {

/// Retrieve or create the CUDA Fortran GPU module in the given \p mod.
mlir::gpu::GPUModuleOp getOrCreateGPUModule(mlir::ModuleOp mod,
                                            mlir::SymbolTable &symTab);

bool isCUDADeviceContext(mlir::Operation *op);
bool isCUDADeviceContext(mlir::Region &,
                         bool isDoConcurrentOffloadEnabled = false);
bool isRegisteredDeviceGlobal(fir::GlobalOp op);
bool isRegisteredDeviceAttr(std::optional<cuf::DataAttribute> attr);

void genPointerSync(const mlir::Value box, fir::FirOpBuilder &builder);

} // namespace cuf

#endif // FORTRAN_OPTIMIZER_TRANSFORMS_CUFCOMMON_H_
