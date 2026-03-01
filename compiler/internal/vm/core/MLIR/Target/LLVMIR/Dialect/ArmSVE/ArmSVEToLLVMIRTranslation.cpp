//======- ArmSVEToLLVMIRTranslation.cpp - Translate ArmSVE to LLVM IR -=======//
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
// This file implements a translation between the ArmSVE dialect and LLVM IR.
//
//===----------------------------------------------------------------------===//

#include "mlir/Target/LLVMIR/Dialect/ArmSVE/ArmSVEToLLVMIRTranslation.h"
#include "mlir/Dialect/ArmSVE/IR/ArmSVEDialect.h"
#include "mlir/IR/Operation.h"
#include "mlir/Target/LLVMIR/ModuleTranslation.h"

#include "vm/core/IR/IRBuilder.h"
#include "vm/core/IR/IntrinsicsAArch64.h"

using namespace mlir;
using namespace mlir::LLVM;

namespace {
/// Implementation of the dialect interface that converts operations belonging
/// to the ArmSVE dialect to LLVM IR.
class ArmSVEDialectLLVMIRTranslationInterface
    : public LLVMTranslationDialectInterface {
public:
  using LLVMTranslationDialectInterface::LLVMTranslationDialectInterface;

  /// Translates the given operation to LLVM IR using the provided IR builder
  /// and saving the state in `moduleTranslation`.
  LogicalResult
  convertOperation(Operation *op, toolchain::IRBuilderBase &builder,
                   LLVM::ModuleTranslation &moduleTranslation) const final {
    Operation &opInst = *op;
#include "mlir/Dialect/ArmSVE/IR/ArmSVEConversions.inc"

    return failure();
  }
};
} // namespace

void mlir::registerArmSVEDialectTranslation(DialectRegistry &registry) {
  registry.insert<arm_sve::ArmSVEDialect>();
  registry.addExtension(+[](MLIRContext *ctx, arm_sve::ArmSVEDialect *dialect) {
    dialect->addInterfaces<ArmSVEDialectLLVMIRTranslationInterface>();
  });
}

void mlir::registerArmSVEDialectTranslation(MLIRContext &context) {
  DialectRegistry registry;
  registerArmSVEDialectTranslation(registry);
  context.appendDialectRegistry(registry);
}
