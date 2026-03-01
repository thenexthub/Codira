//===-- XeVMToLLVMIRTranslation.cpp - Translate XeVM to LLVM IR -*- C++ -*-===//
//
// This file is licensed under the Apache License v2.0 with LLVM Exceptions.
// See https://toolchain.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements a translation between the MLIR XeVM dialect and
// LLVM IR.
//
//===----------------------------------------------------------------------===//

#include "mlir/Target/LLVMIR/Dialect/XeVM/XeVMToLLVMIRTranslation.h"
#include "mlir/Dialect/LLVMIR/XeVMDialect.h"
#include "mlir/IR/BuiltinAttributes.h"
#include "mlir/IR/Operation.h"
#include "mlir/Target/LLVMIR/ModuleTranslation.h"

#include "vm/core/ADT/TypeSwitch.h"
#include "vm/core/IR/Constants.h"
#include "vm/core/IR/LLVMContext.h"
#include "vm/core/IR/Metadata.h"

#include "vm/core/IR/ConstantRange.h"
#include "vm/core/IR/IRBuilder.h"
#include "vm/core/Support/raw_ostream.h"

using namespace mlir;
using namespace mlir::LLVM;

namespace {
/// Implementation of the dialect interface that converts operations belonging
/// to the XeVM dialect to LLVM IR.
class XeVMDialectLLVMIRTranslationInterface
    : public LLVMTranslationDialectInterface {
public:
  using LLVMTranslationDialectInterface::LLVMTranslationDialectInterface;

  /// Attaches module-level metadata for functions marked as kernels.
  LogicalResult
  amendOperation(Operation *op, ArrayRef<toolchain::Instruction *> instructions,
                 NamedAttribute attribute,
                 LLVM::ModuleTranslation &moduleTranslation) const final {
    StringRef attrName = attribute.getName().getValue();
    if (attrName == mlir::xevm::XeVMDialect::getCacheControlsAttrName()) {
      auto cacheControlsArray = dyn_cast<ArrayAttr>(attribute.getValue());
      if (cacheControlsArray.size() != 2) {
        return op->emitOpError(
            "Expected both L1 and L3 cache control attributes!");
      }
      if (instructions.size() != 1) {
        return op->emitOpError("Expecting a single instruction");
      }
      return handleDecorationCacheControl(instructions.front(),
                                          cacheControlsArray.getValue());
    }
    return success();
  }

private:
  static LogicalResult handleDecorationCacheControl(toolchain::Instruction *inst,
                                                    ArrayRef<Attribute> attrs) {
    SmallVector<toolchain::Metadata *> decorations;
    toolchain::LLVMContext &ctx = inst->getContext();
    toolchain::Type *i32Ty = toolchain::IntegerType::getInt32Ty(ctx);
    toolchain::transform(
        attrs, std::back_inserter(decorations),
        [&ctx, i32Ty](Attribute attr) -> toolchain::Metadata * {
          auto valuesArray = dyn_cast<ArrayAttr>(attr).getValue();
          std::array<toolchain::Metadata *, 4> metadata;
          toolchain::transform(
              valuesArray, metadata.begin(), [i32Ty](Attribute valueAttr) {
                return toolchain::ConstantAsMetadata::get(toolchain::ConstantInt::get(
                    i32Ty, cast<IntegerAttr>(valueAttr).getValue()));
              });
          return toolchain::MDNode::get(ctx, metadata);
        });
    constexpr toolchain::StringLiteral decorationCacheControlMDName =
        "spirv.DecorationCacheControlINTEL";
    inst->setMetadata(decorationCacheControlMDName,
                      toolchain::MDNode::get(ctx, decorations));
    return success();
  }
};
} // namespace

void mlir::registerXeVMDialectTranslation(::mlir::DialectRegistry &registry) {
  registry.insert<xevm::XeVMDialect>();
  registry.addExtension(+[](MLIRContext *ctx, xevm::XeVMDialect *dialect) {
    dialect->addInterfaces<XeVMDialectLLVMIRTranslationInterface>();
  });
}

void mlir::registerXeVMDialectTranslation(::mlir::MLIRContext &context) {
  DialectRegistry registry;
  registerXeVMDialectTranslation(registry);
  context.appendDialectRegistry(registry);
}
