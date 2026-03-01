//===- ComplexToSPIRV.cpp - Complex to SPIR-V Patterns --------------------===//
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
// This file implements patterns to convert Complex dialect to SPIR-V dialect.
//
//===----------------------------------------------------------------------===//

#include "mlir/Conversion/ComplexToSPIRV/ComplexToSPIRV.h"
#include "mlir/Dialect/Complex/IR/Complex.h"
#include "mlir/Dialect/SPIRV/IR/SPIRVOps.h"
#include "mlir/Dialect/SPIRV/Transforms/SPIRVConversion.h"
#include "mlir/Transforms/DialectConversion.h"

#define DEBUG_TYPE "complex-to-spirv-pattern"

using namespace mlir;

//===----------------------------------------------------------------------===//
// Operation conversion
//===----------------------------------------------------------------------===//

namespace {

struct ConstantOpPattern final : OpConversionPattern<complex::ConstantOp> {
  using Base::Base;

  LogicalResult
  matchAndRewrite(complex::ConstantOp constOp, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const override {
    auto spirvType =
        getTypeConverter()->convertType<ShapedType>(constOp.getType());
    if (!spirvType)
      return rewriter.notifyMatchFailure(constOp,
                                         "unable to convert result type");

    rewriter.replaceOpWithNewOp<spirv::ConstantOp>(
        constOp, spirvType,
        DenseElementsAttr::get(spirvType, constOp.getValue().getValue()));
    return success();
  }
};

struct CreateOpPattern final : OpConversionPattern<complex::CreateOp> {
  using Base::Base;

  LogicalResult
  matchAndRewrite(complex::CreateOp createOp, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const override {
    Type spirvType = getTypeConverter()->convertType(createOp.getType());
    if (!spirvType)
      return rewriter.notifyMatchFailure(createOp,
                                         "unable to convert result type");

    rewriter.replaceOpWithNewOp<spirv::CompositeConstructOp>(
        createOp, spirvType, adaptor.getOperands());
    return success();
  }
};

struct ReOpPattern final : OpConversionPattern<complex::ReOp> {
  using Base::Base;

  LogicalResult
  matchAndRewrite(complex::ReOp reOp, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const override {
    Type spirvType = getTypeConverter()->convertType(reOp.getType());
    if (!spirvType)
      return rewriter.notifyMatchFailure(reOp, "unable to convert result type");

    rewriter.replaceOpWithNewOp<spirv::CompositeExtractOp>(
        reOp, adaptor.getComplex(), toolchain::ArrayRef(0));
    return success();
  }
};

struct ImOpPattern final : OpConversionPattern<complex::ImOp> {
  using Base::Base;

  LogicalResult
  matchAndRewrite(complex::ImOp imOp, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const override {
    Type spirvType = getTypeConverter()->convertType(imOp.getType());
    if (!spirvType)
      return rewriter.notifyMatchFailure(imOp, "unable to convert result type");

    rewriter.replaceOpWithNewOp<spirv::CompositeExtractOp>(
        imOp, adaptor.getComplex(), toolchain::ArrayRef(1));
    return success();
  }
};

} // namespace

//===----------------------------------------------------------------------===//
// Pattern population
//===----------------------------------------------------------------------===//

void mlir::populateComplexToSPIRVPatterns(
    const SPIRVTypeConverter &typeConverter, RewritePatternSet &patterns) {
  MLIRContext *context = patterns.getContext();

  patterns.add<ConstantOpPattern, CreateOpPattern, ReOpPattern, ImOpPattern>(
      typeConverter, context);
}
