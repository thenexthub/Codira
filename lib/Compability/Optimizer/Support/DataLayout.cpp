//===-- Optimizer/Support/DataLayout.cpp ----------------------------------===//
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

#include "language/Compability/Optimizer/Support/DataLayout.h"
#include "language/Compability/Optimizer/Dialect/Support/FIRContext.h"
#include "language/Compability/Optimizer/Support/FatalError.h"
#include "mlir/Dialect/DLTI/DLTI.h"
#include "mlir/Dialect/GPU/IR/GPUDialect.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/Interfaces/DataLayoutInterfaces.h"
#include "mlir/Support/LLVM.h"
#include "mlir/Target/LLVMIR/Import.h"
#include "toolchain/IR/DataLayout.h"
#include "toolchain/MC/TargetRegistry.h"
#include "toolchain/Support/TargetSelect.h"
#include "toolchain/Target/TargetMachine.h"

namespace {
template <typename ModOpTy>
static void setDataLayout(ModOpTy mlirModule, const toolchain::DataLayout &dl) {
  mlir::MLIRContext *context = mlirModule.getContext();
  mlirModule->setAttr(
      mlir::LLVM::LLVMDialect::getDataLayoutAttrName(),
      mlir::StringAttr::get(context, dl.getStringRepresentation()));
  mlir::DataLayoutSpecInterface dlSpec = mlir::translateDataLayout(dl, context);
  mlirModule->setAttr(mlir::DLTIDialect::kDataLayoutAttrName, dlSpec);
}

template <typename ModOpTy>
static void setDataLayoutFromAttributes(ModOpTy mlirModule,
                                        bool allowDefaultLayout) {
  if (mlirModule.getDataLayoutSpec())
    return; // Already set.
  if (auto dataLayoutString =
          mlirModule->template getAttrOfType<mlir::StringAttr>(
              mlir::LLVM::LLVMDialect::getDataLayoutAttrName())) {
    toolchain::DataLayout toolchainDataLayout(dataLayoutString);
    fir::support::setMLIRDataLayout(mlirModule, toolchainDataLayout);
    return;
  }
  if (!allowDefaultLayout)
    return;
  toolchain::DataLayout toolchainDataLayout("");
  fir::support::setMLIRDataLayout(mlirModule, toolchainDataLayout);
}

template <typename ModOpTy>
static std::optional<mlir::DataLayout>
getOrSetDataLayout(ModOpTy mlirModule, bool allowDefaultLayout) {
  if (!mlirModule.getDataLayoutSpec())
    fir::support::setMLIRDataLayoutFromAttributes(mlirModule,
                                                  allowDefaultLayout);
  if (!mlirModule.getDataLayoutSpec() &&
      !mlir::isa<mlir::gpu::GPUModuleOp>(mlirModule))
    return std::nullopt;
  return mlir::DataLayout(mlirModule);
}

} // namespace

void fir::support::setMLIRDataLayout(mlir::ModuleOp mlirModule,
                                     const toolchain::DataLayout &dl) {
  setDataLayout(mlirModule, dl);
}

void fir::support::setMLIRDataLayout(mlir::gpu::GPUModuleOp mlirModule,
                                     const toolchain::DataLayout &dl) {
  setDataLayout(mlirModule, dl);
}

void fir::support::setMLIRDataLayoutFromAttributes(mlir::ModuleOp mlirModule,
                                                   bool allowDefaultLayout) {
  setDataLayoutFromAttributes(mlirModule, allowDefaultLayout);
}

void fir::support::setMLIRDataLayoutFromAttributes(
    mlir::gpu::GPUModuleOp mlirModule, bool allowDefaultLayout) {
  setDataLayoutFromAttributes(mlirModule, allowDefaultLayout);
}

std::optional<mlir::DataLayout>
fir::support::getOrSetMLIRDataLayout(mlir::ModuleOp mlirModule,
                                     bool allowDefaultLayout) {
  return getOrSetDataLayout(mlirModule, allowDefaultLayout);
}

std::optional<mlir::DataLayout>
fir::support::getOrSetMLIRDataLayout(mlir::gpu::GPUModuleOp mlirModule,
                                     bool allowDefaultLayout) {
  return getOrSetDataLayout(mlirModule, allowDefaultLayout);
}
