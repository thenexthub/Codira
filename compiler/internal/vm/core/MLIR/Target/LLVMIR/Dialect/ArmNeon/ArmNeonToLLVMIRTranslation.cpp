//===- ArmNeonToLLVMIRTranslation.cpp - Translate ArmNeon to LLVM IR ------===//
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
// This file implements a translation between the MLIR ArmNeon dialect and
// LLVM IR.
//
//===----------------------------------------------------------------------===//

#include "mlir/Target/LLVMIR/Dialect/ArmNeon/ArmNeonToLLVMIRTranslation.h"
#include "mlir/Dialect/ArmNeon/ArmNeonDialect.h"
#include "mlir/IR/Operation.h"
#include "mlir/Target/LLVMIR/ModuleTranslation.h"

#include "vm/core/IR/IRBuilder.h"
#include "vm/core/IR/IntrinsicsAArch64.h"

using namespace mlir;
using namespace mlir::LLVM;

namespace {
/// Implementation of the dialect interface that converts operations belonging
/// to the ArmNeon dialect to LLVM IR.
class ArmNeonDialectLLVMIRTranslationInterface
    : public LLVMTranslationDialectInterface {
public:
  using LLVMTranslationDialectInterface::LLVMTranslationDialectInterface;

  /// Translates the given operation to LLVM IR using the provided IR builder
  /// and saving the state in `moduleTranslation`.
  LogicalResult
  convertOperation(Operation *op, toolchain::IRBuilderBase &builder,
                   LLVM::ModuleTranslation &moduleTranslation) const final {
    Operation &opInst = *op;
#include "mlir/Dialect/ArmNeon/ArmNeonConversions.inc"

    return failure();
  }
};
} // namespace

void mlir::registerArmNeonDialectTranslation(DialectRegistry &registry) {
  registry.insert<arm_neon::ArmNeonDialect>();
  registry.addExtension(
      +[](MLIRContext *ctx, arm_neon::ArmNeonDialect *dialect) {
        dialect->addInterfaces<ArmNeonDialectLLVMIRTranslationInterface>();
      });
}

void mlir::registerArmNeonDialectTranslation(MLIRContext &context) {
  DialectRegistry registry;
  registerArmNeonDialectTranslation(registry);
  context.appendDialectRegistry(registry);
}
