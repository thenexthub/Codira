//===- UBToSPIRV.cpp - UB to SPIRV-V dialect conversion -------------------===//
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

#include "mlir/Conversion/UBToSPIRV/UBToSPIRV.h"

#include "mlir/Dialect/SPIRV/IR/SPIRVDialect.h"
#include "mlir/Dialect/SPIRV/Transforms/SPIRVConversion.h"
#include "mlir/Dialect/UB/IR/UBOps.h"
#include "mlir/Pass/Pass.h"

namespace mlir {
#define GEN_PASS_DEF_UBTOSPIRVCONVERSIONPASS
#include "mlir/Conversion/Passes.h.inc"
} // namespace mlir

using namespace mlir;

namespace {

struct PoisonOpLowering final : OpConversionPattern<ub::PoisonOp> {
  using Base::Base;

  LogicalResult
  matchAndRewrite(ub::PoisonOp op, OpAdaptor,
                  ConversionPatternRewriter &rewriter) const override {
    Type origType = op.getType();
    Type resType = getTypeConverter()->convertType(origType);
    if (!resType)
      return rewriter.notifyMatchFailure(op, [&](Diagnostic &diag) {
        diag << "failed to convert result type " << origType;
      });

    rewriter.replaceOpWithNewOp<spirv::UndefOp>(op, resType);
    return success();
  }
};

struct UnreachableOpLowering final : OpConversionPattern<ub::UnreachableOp> {
  using Base::Base;

  LogicalResult
  matchAndRewrite(ub::UnreachableOp op, OpAdaptor,
                  ConversionPatternRewriter &rewriter) const override {
    rewriter.replaceOpWithNewOp<spirv::UnreachableOp>(op);
    return success();
  }
};

} // namespace

//===----------------------------------------------------------------------===//
// Pass Definition
//===----------------------------------------------------------------------===//

namespace {
struct UBToSPIRVConversionPass final
    : impl::UBToSPIRVConversionPassBase<UBToSPIRVConversionPass> {
  using Base::Base;

  void runOnOperation() override {
    Operation *op = getOperation();
    spirv::TargetEnvAttr targetAttr = spirv::lookupTargetEnvOrDefault(op);
    std::unique_ptr<SPIRVConversionTarget> target =
        SPIRVConversionTarget::get(targetAttr);

    SPIRVConversionOptions options;
    SPIRVTypeConverter typeConverter(targetAttr, options);

    RewritePatternSet patterns(&getContext());
    ub::populateUBToSPIRVConversionPatterns(typeConverter, patterns);

    if (failed(applyPartialConversion(op, *target, std::move(patterns))))
      signalPassFailure();
  }
};
} // namespace

//===----------------------------------------------------------------------===//
// Pattern Population
//===----------------------------------------------------------------------===//

void mlir::ub::populateUBToSPIRVConversionPatterns(
    const SPIRVTypeConverter &converter, RewritePatternSet &patterns) {
  patterns.add<PoisonOpLowering, UnreachableOpLowering>(converter,
                                                        patterns.getContext());
}
