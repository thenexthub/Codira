//===- GPUToLLVMIRTranslation.cpp - Translate GPU dialect to LLVM IR ------===//
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
// This file implements a translation between the MLIR GPU dialect and LLVM IR.
//
//===----------------------------------------------------------------------===//
#include "mlir/Target/LLVMIR/Dialect/GPU/GPUToLLVMIRTranslation.h"
#include "mlir/Dialect/GPU/IR/GPUDialect.h"
#include "mlir/Target/LLVMIR/LLVMTranslationInterface.h"
#include "vm/core/ADT/TypeSwitch.h"

using namespace mlir;

namespace {
LogicalResult launchKernel(gpu::LaunchFuncOp launchOp,
                           toolchain::IRBuilderBase &builder,
                           LLVM::ModuleTranslation &moduleTranslation) {
  auto kernelBinary = SymbolTable::lookupNearestSymbolFrom<gpu::BinaryOp>(
      launchOp, launchOp.getKernelModuleName());
  if (!kernelBinary) {
    launchOp.emitError("Couldn't find the binary holding the kernel: ")
        << launchOp.getKernelModuleName();
    return failure();
  }
  auto offloadingHandler =
      dyn_cast<gpu::OffloadingLLVMTranslationAttrInterface>(
          kernelBinary.getOffloadingHandlerAttr());
  assert(offloadingHandler && "Invalid offloading handler.");
  return offloadingHandler.launchKernel(launchOp, kernelBinary, builder,
                                        moduleTranslation);
}

class GPUDialectLLVMIRTranslationInterface
    : public LLVMTranslationDialectInterface {
public:
  using LLVMTranslationDialectInterface::LLVMTranslationDialectInterface;

  LogicalResult
  convertOperation(Operation *operation, toolchain::IRBuilderBase &builder,
                   LLVM::ModuleTranslation &moduleTranslation) const override {
    return toolchain::TypeSwitch<Operation *, LogicalResult>(operation)
        .Case([&](gpu::GPUModuleOp) { return success(); })
        .Case([&](gpu::BinaryOp op) {
          auto offloadingHandler =
              dyn_cast<gpu::OffloadingLLVMTranslationAttrInterface>(
                  op.getOffloadingHandlerAttr());
          assert(offloadingHandler && "Invalid offloading handler.");
          return offloadingHandler.embedBinary(op, builder, moduleTranslation);
        })
        .Case([&](gpu::LaunchFuncOp op) {
          return launchKernel(op, builder, moduleTranslation);
        })
        .Default([&](Operation *op) {
          return op->emitError("unsupported GPU operation: ") << op->getName();
        });
  }
};

} // namespace

void mlir::registerGPUDialectTranslation(DialectRegistry &registry) {
  registry.insert<gpu::GPUDialect>();
  registry.addExtension(+[](MLIRContext *ctx, gpu::GPUDialect *dialect) {
    dialect->addInterfaces<GPUDialectLLVMIRTranslationInterface>();
  });
}

void mlir::registerGPUDialectTranslation(MLIRContext &context) {
  DialectRegistry registry;
  registerGPUDialectTranslation(registry);
  context.appendDialectRegistry(registry);
}
