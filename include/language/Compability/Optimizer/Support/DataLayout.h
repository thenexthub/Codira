//===-- Optimizer/Support/DataLayout.h --------------------------*- C++ -*-===//
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
//
// Coding style: https://mlir.toolchain.org/getting_started/DeveloperGuide/
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_COMPABILITY_OPTIMIZER_SUPPORT_DATALAYOUT_H
#define LANGUAGE_COMPABILITY_OPTIMIZER_SUPPORT_DATALAYOUT_H

#include "mlir/Interfaces/DataLayoutInterfaces.h"
#include <optional>

namespace mlir {
class ModuleOp;
namespace gpu {
class GPUModuleOp;
} // namespace gpu
} // namespace mlir

namespace toolchain {
class DataLayout;
} // namespace toolchain

namespace fir::support {
/// Create an mlir::DataLayoutSpecInterface attribute from an toolchain::DataLayout
/// and set it on the provided mlir::ModuleOp.
/// Also set the toolchain.data_layout attribute with the string representation of
/// the toolchain::DataLayout on the module.
/// These attributes are replaced if they were already set.
void setMLIRDataLayout(mlir::ModuleOp mlirModule, const toolchain::DataLayout &dl);
void setMLIRDataLayout(mlir::gpu::GPUModuleOp mlirModule,
                       const toolchain::DataLayout &dl);

/// Create an mlir::DataLayoutSpecInterface from the toolchain.data_layout attribute
/// if one is provided. If such attribute is not available, create a default
/// target independent layout when allowDefaultLayout is true. Otherwise do
/// nothing.
void setMLIRDataLayoutFromAttributes(mlir::ModuleOp mlirModule,
                                     bool allowDefaultLayout);
void setMLIRDataLayoutFromAttributes(mlir::gpu::GPUModuleOp mlirModule,
                                     bool allowDefaultLayout);

/// Create mlir::DataLayout from the data layout information on the
/// mlir::Module. Creates the data layout information attributes with
/// setMLIRDataLayoutFromAttributes if the DLTI attribute is not yet set. If no
/// information is present at all and \p allowDefaultLayout is false, returns
/// std::nullopt.
std::optional<mlir::DataLayout>
getOrSetMLIRDataLayout(mlir::ModuleOp mlirModule,
                       bool allowDefaultLayout = false);
std::optional<mlir::DataLayout>
getOrSetMLIRDataLayout(mlir::gpu::GPUModuleOp mlirModule,
                       bool allowDefaultLayout = false);

} // namespace fir::support

#endif // FORTRAN_OPTIMIZER_SUPPORT_DATALAYOUT_H
