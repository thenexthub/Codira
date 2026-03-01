//===- VCIXToLLVMIRTranslation.cpp - Translate VCIX to LLVM IR ------------===//
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
// This file implements a translation between the MLIR VCIX dialect and
// LLVM IR.
//
//===----------------------------------------------------------------------===//

#include "mlir/Target/LLVMIR/Dialect/VCIX/VCIXToLLVMIRTranslation.h"
#include "mlir/Dialect/LLVMIR/VCIXDialect.h"
#include "mlir/IR/BuiltinAttributes.h"
#include "mlir/IR/Operation.h"
#include "mlir/Target/LLVMIR/ModuleTranslation.h"

#include "vm/core/IR/IRBuilder.h"
#include "vm/core/IR/IntrinsicsRISCV.h"

using namespace mlir;
using namespace mlir::LLVM;
using mlir::LLVM::detail::createIntrinsicCall;

/// Infer XLen type from opcode's type. This is done to avoid passing target
/// option around.
static toolchain::Type *getXlenType(Attribute opcodeAttr,
                               LLVM::ModuleTranslation &moduleTranslation) {
  auto intAttr = cast<IntegerAttr>(opcodeAttr);
  unsigned xlenWidth = cast<IntegerType>(intAttr.getType()).getWidth();
  return toolchain::Type::getIntNTy(moduleTranslation.getLLVMContext(), xlenWidth);
}

/// Return VL for VCIX intrinsic. If vl was previously set, return it,
/// otherwise construct a constant using fixed vector type.
static toolchain::Value *createVL(toolchain::IRBuilderBase &builder, toolchain::Value *vl,
                             VectorType vtype, toolchain::Type *xlen, Location loc,
                             LLVM::ModuleTranslation &moduleTranslation) {
  if (vl) {
    assert(vtype.isScalable() &&
           "vl parameter must be set for scalable vectors");
    return builder.CreateZExtOrTrunc(vl, xlen);
  }

  assert(vtype.getRank() == 1 && "Only 1-d fixed vectors are supported");
  return mlir::LLVM::detail::getLLVMConstant(
      xlen,
      IntegerAttr::get(IntegerType::get(&moduleTranslation.getContext(), 64),
                       vtype.getShape()[0]),
      loc, moduleTranslation);
}

namespace {
/// Implementation of the dialect interface that converts operations belonging
/// to the VCIX dialect to LLVM IR.
class VCIXDialectLLVMIRTranslationInterface
    : public LLVMTranslationDialectInterface {
public:
  using LLVMTranslationDialectInterface::LLVMTranslationDialectInterface;

  /// Translates the given operation to LLVM IR using the provided IR builder
  /// and saving the state in `moduleTranslation`.
  LogicalResult
  convertOperation(Operation *op, toolchain::IRBuilderBase &builder,
                   LLVM::ModuleTranslation &moduleTranslation) const final {
    Operation &opInst = *op;
#include "mlir/Dialect/LLVMIR/VCIXConversions.inc"

    return failure();
  }
};
} // namespace

void mlir::registerVCIXDialectTranslation(DialectRegistry &registry) {
  registry.insert<vcix::VCIXDialect>();
  registry.addExtension(+[](MLIRContext *ctx, vcix::VCIXDialect *dialect) {
    dialect->addInterfaces<VCIXDialectLLVMIRTranslationInterface>();
  });
}

void mlir::registerVCIXDialectTranslation(MLIRContext &context) {
  DialectRegistry registry;
  registerVCIXDialectTranslation(registry);
  context.appendDialectRegistry(registry);
}
