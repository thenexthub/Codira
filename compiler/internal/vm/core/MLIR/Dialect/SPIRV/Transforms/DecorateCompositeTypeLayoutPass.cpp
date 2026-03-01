//===- DecorateCompositeTypeLayoutPass.cpp - Decorate composite type ------===//
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
// This file implements a pass to decorate the composite types used by
// composite objects in the StorageBuffer, PhysicalStorageBuffer, Uniform, and
// PushConstant storage classes with layout information. See SPIR-V spec
// "2.16.2. Validation Rules for Shader Capabilities" for more details.
//
//===----------------------------------------------------------------------===//

#include "mlir/Dialect/SPIRV/Transforms/Passes.h"

#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/SPIRV/IR/SPIRVDialect.h"
#include "mlir/Dialect/SPIRV/IR/SPIRVOps.h"
#include "mlir/Dialect/SPIRV/Utils/LayoutUtils.h"
#include "mlir/Transforms/DialectConversion.h"

#include "vm/core/Support/FormatVariadic.h"

using namespace mlir;

namespace mlir {
namespace spirv {
#define GEN_PASS_DEF_SPIRVCOMPOSITETYPELAYOUTPASS
#include "mlir/Dialect/SPIRV/Transforms/Passes.h.inc"
} // namespace spirv
} // namespace mlir

namespace {
class SPIRVGlobalVariableOpLayoutInfoDecoration
    : public OpRewritePattern<spirv::GlobalVariableOp> {
public:
  using Base::Base;

  LogicalResult matchAndRewrite(spirv::GlobalVariableOp op,
                                PatternRewriter &rewriter) const override {
    SmallVector<NamedAttribute, 4> globalVarAttrs;

    auto ptrType = cast<spirv::PointerType>(op.getType());
    auto pointeeType = cast<spirv::StructType>(ptrType.getPointeeType());
    spirv::StructType structType = VulkanLayoutUtils::decorateType(pointeeType);

    if (!structType)
      return op->emitError(toolchain::formatv(
          "failed to decorate (unsuported pointee type: '{0}')", pointeeType));

    auto decoratedType =
        spirv::PointerType::get(structType, ptrType.getStorageClass());

    // Save all named attributes except "type" attribute.
    for (const auto &attr : op->getAttrs()) {
      if (attr.getName() == "type")
        continue;
      globalVarAttrs.push_back(attr);
    }

    rewriter.replaceOpWithNewOp<spirv::GlobalVariableOp>(
        op, TypeAttr::get(decoratedType), globalVarAttrs);
    return success();
  }
};

class SPIRVAddressOfOpLayoutInfoDecoration
    : public OpRewritePattern<spirv::AddressOfOp> {
public:
  using Base::Base;

  LogicalResult matchAndRewrite(spirv::AddressOfOp op,
                                PatternRewriter &rewriter) const override {
    auto spirvModule = op->getParentOfType<spirv::ModuleOp>();
    auto varName = op.getVariableAttr();
    auto varOp = spirvModule.lookupSymbol<spirv::GlobalVariableOp>(varName);

    rewriter.replaceOpWithNewOp<spirv::AddressOfOp>(
        op, varOp.getType(), SymbolRefAttr::get(varName.getAttr()));
    return success();
  }
};

template <typename OpT>
class SPIRVPassThroughConversion : public OpConversionPattern<OpT> {
public:
  using OpConversionPattern<OpT>::OpConversionPattern;

  LogicalResult
  matchAndRewrite(OpT op, typename OpT::Adaptor adaptor,
                  ConversionPatternRewriter &rewriter) const override {
    rewriter.modifyOpInPlace(op,
                             [&] { op->setOperands(adaptor.getOperands()); });
    return success();
  }
};
} // namespace

static void populateSPIRVLayoutInfoPatterns(RewritePatternSet &patterns) {
  patterns.add<SPIRVGlobalVariableOpLayoutInfoDecoration,
               SPIRVAddressOfOpLayoutInfoDecoration,
               SPIRVPassThroughConversion<spirv::AccessChainOp>,
               SPIRVPassThroughConversion<spirv::LoadOp>,
               SPIRVPassThroughConversion<spirv::StoreOp>>(
      patterns.getContext());
}

namespace {
class DecorateSPIRVCompositeTypeLayoutPass
    : public spirv::impl::SPIRVCompositeTypeLayoutPassBase<
          DecorateSPIRVCompositeTypeLayoutPass> {
  void runOnOperation() override;
};
} // namespace

void DecorateSPIRVCompositeTypeLayoutPass::runOnOperation() {
  auto module = getOperation();
  RewritePatternSet patterns(module.getContext());
  populateSPIRVLayoutInfoPatterns(patterns);
  ConversionTarget target(*(module.getContext()));
  target.addLegalDialect<spirv::SPIRVDialect>();
  target.addLegalOp<func::FuncOp>();
  target.addDynamicallyLegalOp<spirv::GlobalVariableOp>(
      [](spirv::GlobalVariableOp op) {
        return VulkanLayoutUtils::isLegalType(op.getType());
      });

  // Change the type for the direct users.
  target.addDynamicallyLegalOp<spirv::AddressOfOp>([](spirv::AddressOfOp op) {
    return VulkanLayoutUtils::isLegalType(op.getPointer().getType());
  });

  // Change the type for the indirect users.
  target.addDynamicallyLegalOp<spirv::AccessChainOp, spirv::LoadOp,
                               spirv::StoreOp>([&](Operation *op) {
    for (Value operand : op->getOperands()) {
      auto addrOp = operand.getDefiningOp<spirv::AddressOfOp>();
      if (addrOp &&
          !VulkanLayoutUtils::isLegalType(addrOp.getPointer().getType()))
        return false;
    }
    return true;
  });

  FrozenRewritePatternSet frozenPatterns(std::move(patterns));
  for (auto spirvModule : module.getOps<spirv::ModuleOp>())
    if (failed(applyFullConversion(spirvModule, target, frozenPatterns)))
      signalPassFailure();
}
