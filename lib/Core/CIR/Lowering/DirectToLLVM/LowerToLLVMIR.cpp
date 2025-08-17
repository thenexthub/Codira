//===----------------------------------------------------------------------===//
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
// This file implements lowering of CIR attributes and operations directly to
// LLVMIR.
//
//===----------------------------------------------------------------------===//

#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/IR/DialectRegistry.h"
#include "mlir/Target/LLVMIR/LLVMTranslationInterface.h"
#include "mlir/Target/LLVMIR/ModuleTranslation.h"
#include "language/Core/CIR/Dialect/IR/CIRAttrs.h"
#include "language/Core/CIR/Dialect/IR/CIRDialect.h"
#include "language/Core/CIR/MissingFeatures.h"
#include "toolchain/ADT/ArrayRef.h"
#include "toolchain/IR/Constant.h"
#include "toolchain/IR/GlobalVariable.h"

using namespace toolchain;

namespace cir {
namespace direct {

/// Implementation of the dialect interface that converts CIR attributes to LLVM
/// IR metadata.
class CIRDialectLLVMIRTranslationInterface
    : public mlir::LLVMTranslationDialectInterface {
public:
  using LLVMTranslationDialectInterface::LLVMTranslationDialectInterface;

  /// Translates the given operation to LLVM IR using the provided IR builder
  /// and saving the state in `moduleTranslation`.
  mlir::LogicalResult convertOperation(
      mlir::Operation *op, toolchain::IRBuilderBase &builder,
      mlir::LLVM::ModuleTranslation &moduleTranslation) const final {

    if (auto cirOp = toolchain::dyn_cast<mlir::LLVM::ZeroOp>(op))
      moduleTranslation.mapValue(cirOp.getResult()) =
          toolchain::Constant::getNullValue(
              moduleTranslation.convertType(cirOp.getType()));

    return mlir::success();
  }
};

void registerCIRDialectTranslation(mlir::DialectRegistry &registry) {
  registry.insert<cir::CIRDialect>();
  registry.addExtension(+[](mlir::MLIRContext *ctx, cir::CIRDialect *dialect) {
    dialect->addInterfaces<CIRDialectLLVMIRTranslationInterface>();
  });
}

} // namespace direct
} // namespace cir

namespace mlir {
void registerCIRDialectTranslation(mlir::MLIRContext &context) {
  mlir::DialectRegistry registry;
  cir::direct::registerCIRDialectTranslation(registry);
  context.appendDialectRegistry(registry);
}
} // namespace mlir
