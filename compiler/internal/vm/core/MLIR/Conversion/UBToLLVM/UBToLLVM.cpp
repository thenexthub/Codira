//===- UBToLLVM.cpp - UB to LLVM dialect conversion -----------------------===//
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

#include "mlir/Conversion/UBToLLVM/UBToLLVM.h"

#include "mlir/Conversion/ConvertToLLVM/ToLLVMInterface.h"
#include "mlir/Conversion/LLVMCommon/ConversionTarget.h"
#include "mlir/Conversion/LLVMCommon/Pattern.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/Dialect/UB/IR/UBOps.h"
#include "mlir/IR/TypeUtilities.h"
#include "mlir/Pass/Pass.h"

namespace mlir {
#define GEN_PASS_DEF_UBTOLLVMCONVERSIONPASS
#include "mlir/Conversion/Passes.h.inc"
} // namespace mlir

using namespace mlir;

//===----------------------------------------------------------------------===//
// PoisonOpLowering
//===----------------------------------------------------------------------===//

namespace {
struct PoisonOpLowering : public ConvertOpToLLVMPattern<ub::PoisonOp> {
  using ConvertOpToLLVMPattern::ConvertOpToLLVMPattern;

  LogicalResult
  matchAndRewrite(ub::PoisonOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const override;
};
} // namespace

LogicalResult
PoisonOpLowering::matchAndRewrite(ub::PoisonOp op, OpAdaptor adaptor,
                                  ConversionPatternRewriter &rewriter) const {
  if (!isa<ub::PoisonAttr>(op.getValue())) {
    return rewriter.notifyMatchFailure(op, [&](Diagnostic &diag) {
      diag << "pattern can only convert op with '"
           << ub::PoisonAttr::getMnemonic() << "' poison value";
    });
  }

  Type resType = getTypeConverter()->convertType(op.getType());
  if (!resType) {
    return rewriter.notifyMatchFailure(op, [&](Diagnostic &diag) {
      diag << "failed to convert result type " << op.getType();
    });
  }

  rewriter.replaceOpWithNewOp<LLVM::PoisonOp>(op, resType);
  return success();
}

//===----------------------------------------------------------------------===//
// UnreachableOpLowering
//===----------------------------------------------------------------------===//

namespace {
struct UnreachableOpLowering
    : public ConvertOpToLLVMPattern<ub::UnreachableOp> {
  using ConvertOpToLLVMPattern::ConvertOpToLLVMPattern;

  LogicalResult
  matchAndRewrite(ub::UnreachableOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const override;
};
} // namespace
LogicalResult

UnreachableOpLowering::matchAndRewrite(
    ub::UnreachableOp op, OpAdaptor adaptor,
    ConversionPatternRewriter &rewriter) const {
  rewriter.replaceOpWithNewOp<LLVM::UnreachableOp>(op);
  return success();
}

//===----------------------------------------------------------------------===//
// Pass Definition
//===----------------------------------------------------------------------===//

namespace {
struct UBToLLVMConversionPass
    : public impl::UBToLLVMConversionPassBase<UBToLLVMConversionPass> {
  using Base::Base;

  void runOnOperation() override {
    LLVMConversionTarget target(getContext());
    RewritePatternSet patterns(&getContext());

    LowerToLLVMOptions options(&getContext());
    if (indexBitwidth != kDeriveIndexBitwidthFromDataLayout)
      options.overrideIndexBitwidth(indexBitwidth);

    LLVMTypeConverter converter(&getContext(), options);
    mlir::ub::populateUBToLLVMConversionPatterns(converter, patterns);

    if (failed(applyPartialConversion(getOperation(), target,
                                      std::move(patterns))))
      signalPassFailure();
  }
};
} // namespace

//===----------------------------------------------------------------------===//
// Pattern Population
//===----------------------------------------------------------------------===//

void mlir::ub::populateUBToLLVMConversionPatterns(
    const LLVMTypeConverter &converter, RewritePatternSet &patterns) {
  patterns.add<PoisonOpLowering, UnreachableOpLowering>(converter);
}

//===----------------------------------------------------------------------===//
// ConvertToLLVMPatternInterface implementation
//===----------------------------------------------------------------------===//

namespace {
/// Implement the interface to convert UB to LLVM.
struct UBToLLVMDialectInterface : public ConvertToLLVMPatternInterface {
  using ConvertToLLVMPatternInterface::ConvertToLLVMPatternInterface;
  void loadDependentDialects(MLIRContext *context) const final {
    context->loadDialect<LLVM::LLVMDialect>();
  }

  /// Hook for derived dialect interface to provide conversion patterns
  /// and mark dialect legal for the conversion target.
  void populateConvertToLLVMConversionPatterns(
      ConversionTarget &target, LLVMTypeConverter &typeConverter,
      RewritePatternSet &patterns) const final {
    ub::populateUBToLLVMConversionPatterns(typeConverter, patterns);
  }
};
} // namespace

void mlir::ub::registerConvertUBToLLVMInterface(DialectRegistry &registry) {
  registry.addExtension(+[](MLIRContext *ctx, ub::UBDialect *dialect) {
    dialect->addInterfaces<UBToLLVMDialectInterface>();
  });
}
