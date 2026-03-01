//===- ControlFlowToSPIRV.cpp - ControlFlow to SPIR-V Patterns ------------===//
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
// This file implements patterns to convert standard dialect to SPIR-V dialect.
//
//===----------------------------------------------------------------------===//

#include "mlir/Conversion/ControlFlowToSPIRV/ControlFlowToSPIRV.h"
#include "mlir/Dialect/ControlFlow/IR/ControlFlowOps.h"
#include "mlir/Dialect/SPIRV/IR/SPIRVOps.h"
#include "mlir/Dialect/SPIRV/Transforms/SPIRVConversion.h"
#include "mlir/Dialect/SPIRV/Utils/LayoutUtils.h"
#include "mlir/IR/AffineMap.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Transforms/DialectConversion.h"
#include "vm/core/Support/FormatVariadic.h"

#define DEBUG_TYPE "cf-to-spirv-pattern"

using namespace mlir;

/// Legailze target block arguments.
static LogicalResult legalizeBlockArguments(Block &block, Operation *op,
                                            PatternRewriter &rewriter,
                                            const TypeConverter &converter) {
  auto builder = OpBuilder::atBlockBegin(&block);
  for (unsigned i = 0; i < block.getNumArguments(); ++i) {
    BlockArgument arg = block.getArgument(i);
    if (converter.isLegal(arg.getType()))
      continue;
    Type ty = arg.getType();
    Type newTy = converter.convertType(ty);
    if (!newTy) {
      return rewriter.notifyMatchFailure(
          op, toolchain::formatv("failed to legalize type for argument {0})", arg));
    }
    unsigned argNum = arg.getArgNumber();
    Location loc = arg.getLoc();
    Value newArg = block.insertArgument(argNum, newTy, loc);
    Value convertedValue = converter.materializeSourceConversion(
        builder, op->getLoc(), ty, newArg);
    if (!convertedValue) {
      return rewriter.notifyMatchFailure(
          op, toolchain::formatv("failed to cast new argument {0} to type {1})",
                            newArg, ty));
    }
    arg.replaceAllUsesWith(convertedValue);
    block.eraseArgument(argNum + 1);
  }
  return success();
}

//===----------------------------------------------------------------------===//
// Operation conversion
//===----------------------------------------------------------------------===//

namespace {
/// Converts cf.br to spirv.Branch.
struct BranchOpPattern final : OpConversionPattern<cf::BranchOp> {
  using Base::Base;

  LogicalResult
  matchAndRewrite(cf::BranchOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const override {
    if (failed(legalizeBlockArguments(*op.getDest(), op, rewriter,
                                      *getTypeConverter())))
      return failure();

    rewriter.replaceOpWithNewOp<spirv::BranchOp>(op, op.getDest(),
                                                 adaptor.getDestOperands());
    return success();
  }
};

/// Converts cf.cond_br to spirv.BranchConditional.
struct CondBranchOpPattern final : OpConversionPattern<cf::CondBranchOp> {
  using Base::Base;

  LogicalResult
  matchAndRewrite(cf::CondBranchOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const override {
    if (failed(legalizeBlockArguments(*op.getTrueDest(), op, rewriter,
                                      *getTypeConverter())))
      return failure();

    if (failed(legalizeBlockArguments(*op.getFalseDest(), op, rewriter,
                                      *getTypeConverter())))
      return failure();

    rewriter.replaceOpWithNewOp<spirv::BranchConditionalOp>(
        op, adaptor.getCondition(), op.getTrueDest(),
        adaptor.getTrueDestOperands(), op.getFalseDest(),
        adaptor.getFalseDestOperands());
    return success();
  }
};
} // namespace

//===----------------------------------------------------------------------===//
// Pattern population
//===----------------------------------------------------------------------===//

void mlir::cf::populateControlFlowToSPIRVPatterns(
    const SPIRVTypeConverter &typeConverter, RewritePatternSet &patterns) {
  MLIRContext *context = patterns.getContext();

  patterns.add<BranchOpPattern, CondBranchOpPattern>(typeConverter, context);
}
