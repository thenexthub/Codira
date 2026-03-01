//===- FuncToSPIRV.cpp - Func to SPIR-V Patterns ------------------===//
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
// This file implements patterns to convert Func dialect to SPIR-V dialect.
//
//===----------------------------------------------------------------------===//

#include "mlir/Conversion/FuncToSPIRV/FuncToSPIRV.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/SPIRV/IR/SPIRVOps.h"
#include "mlir/Dialect/SPIRV/Transforms/SPIRVConversion.h"
#include "mlir/Dialect/SPIRV/Utils/LayoutUtils.h"
#include "mlir/IR/AffineMap.h"

#define DEBUG_TYPE "func-to-spirv-pattern"

using namespace mlir;

//===----------------------------------------------------------------------===//
// Operation conversion
//===----------------------------------------------------------------------===//

// Note that DRR cannot be used for the patterns in this file: we may need to
// convert type along the way, which requires ConversionPattern. DRR generates
// normal RewritePattern.

namespace {

/// Converts func.return to spirv.Return.
class ReturnOpPattern final : public OpConversionPattern<func::ReturnOp> {
public:
  using Base::Base;

  LogicalResult
  matchAndRewrite(func::ReturnOp returnOp, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const override {
    if (returnOp.getNumOperands() > 1)
      return failure();

    if (returnOp.getNumOperands() == 1) {
      rewriter.replaceOpWithNewOp<spirv::ReturnValueOp>(
          returnOp, adaptor.getOperands()[0]);
    } else {
      rewriter.replaceOpWithNewOp<spirv::ReturnOp>(returnOp);
    }
    return success();
  }
};

/// Converts func.call to spirv.FunctionCall.
class CallOpPattern final : public OpConversionPattern<func::CallOp> {
public:
  using Base::Base;

  LogicalResult
  matchAndRewrite(func::CallOp callOp, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const override {
    // multiple results func was not converted to spirv.func
    if (callOp.getNumResults() > 1)
      return failure();
    if (callOp.getNumResults() == 1) {
      auto resultType =
          getTypeConverter()->convertType(callOp.getResult(0).getType());
      if (!resultType)
        return failure();
      rewriter.replaceOpWithNewOp<spirv::FunctionCallOp>(
          callOp, resultType, adaptor.getOperands(), callOp->getAttrs());
    } else {
      rewriter.replaceOpWithNewOp<spirv::FunctionCallOp>(
          callOp, TypeRange(), adaptor.getOperands(), callOp->getAttrs());
    }
    return success();
  }
};

} // namespace

//===----------------------------------------------------------------------===//
// Pattern population
//===----------------------------------------------------------------------===//

void mlir::populateFuncToSPIRVPatterns(const SPIRVTypeConverter &typeConverter,
                                       RewritePatternSet &patterns) {
  MLIRContext *context = patterns.getContext();

  patterns.add<ReturnOpPattern, CallOpPattern>(typeConverter, context);
}
