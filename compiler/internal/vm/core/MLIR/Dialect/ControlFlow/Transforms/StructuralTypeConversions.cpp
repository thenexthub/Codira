//===- TypeConversion.cpp - Type Conversion of Unstructured Control Flow --===//
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
// This file implements a pass to convert MLIR standard and builtin dialects
// into the LLVM IR dialect.
//
//===----------------------------------------------------------------------===//

#include "mlir/Dialect/ControlFlow/Transforms/StructuralTypeConversions.h"

#include "mlir/Dialect/ControlFlow/IR/ControlFlowOps.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/DialectConversion.h"

using namespace mlir;

namespace {

/// Helper function for converting branch ops. This function converts the
/// signature of the given block. If the new block signature is different from
/// `expectedTypes`, returns "failure".
static FailureOr<Block *> getConvertedBlock(ConversionPatternRewriter &rewriter,
                                            const TypeConverter *converter,
                                            Operation *branchOp, Block *block,
                                            TypeRange expectedTypes) {
  assert(converter && "expected non-null type converter");
  assert(!block->isEntryBlock() && "entry blocks have no predecessors");

  // There is nothing to do if the types already match.
  if (block->getArgumentTypes() == expectedTypes)
    return block;

  // Compute the new block argument types and convert the block.
  std::optional<TypeConverter::SignatureConversion> conversion =
      converter->convertBlockSignature(block);
  if (!conversion)
    return rewriter.notifyMatchFailure(branchOp,
                                       "could not compute block signature");
  if (expectedTypes != conversion->getConvertedTypes())
    return rewriter.notifyMatchFailure(
        branchOp,
        "mismatch between adaptor operand types and computed block signature");
  return rewriter.applySignatureConversion(block, *conversion, converter);
}

/// Flatten the given value ranges into a single vector of values.
static SmallVector<Value> flattenValues(ArrayRef<ValueRange> values) {
  SmallVector<Value> result;
  for (const ValueRange &vals : values)
    toolchain::append_range(result, vals);
  return result;
}

/// Convert the destination block signature (if necessary) and change the
/// operands of the branch op.
struct BranchOpConversion : public OpConversionPattern<cf::BranchOp> {
  using OpConversionPattern<cf::BranchOp>::OpConversionPattern;

  LogicalResult
  matchAndRewrite(cf::BranchOp op, OneToNOpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const override {
    SmallVector<Value> flattenedAdaptor = flattenValues(adaptor.getOperands());
    FailureOr<Block *> convertedBlock =
        getConvertedBlock(rewriter, getTypeConverter(), op, op.getSuccessor(),
                          TypeRange(ValueRange(flattenedAdaptor)));
    if (failed(convertedBlock))
      return failure();
    rewriter.replaceOpWithNewOp<cf::BranchOp>(op, flattenedAdaptor,
                                              *convertedBlock);
    return success();
  }
};

/// Convert the destination block signatures (if necessary) and change the
/// operands of the branch op.
struct CondBranchOpConversion : public OpConversionPattern<cf::CondBranchOp> {
  using OpConversionPattern<cf::CondBranchOp>::OpConversionPattern;

  LogicalResult
  matchAndRewrite(cf::CondBranchOp op, OneToNOpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const override {
    SmallVector<Value> flattenedAdaptorTrue =
        flattenValues(adaptor.getTrueDestOperands());
    SmallVector<Value> flattenedAdaptorFalse =
        flattenValues(adaptor.getFalseDestOperands());
    if (!toolchain::hasSingleElement(adaptor.getCondition()))
      return rewriter.notifyMatchFailure(op,
                                         "expected single element condition");
    FailureOr<Block *> convertedTrueBlock =
        getConvertedBlock(rewriter, getTypeConverter(), op, op.getTrueDest(),
                          TypeRange(ValueRange(flattenedAdaptorTrue)));
    if (failed(convertedTrueBlock))
      return failure();
    FailureOr<Block *> convertedFalseBlock =
        getConvertedBlock(rewriter, getTypeConverter(), op, op.getFalseDest(),
                          TypeRange(ValueRange(flattenedAdaptorFalse)));
    if (failed(convertedFalseBlock))
      return failure();
    rewriter.replaceOpWithNewOp<cf::CondBranchOp>(
        op, toolchain::getSingleElement(adaptor.getCondition()),
        flattenedAdaptorTrue, flattenedAdaptorFalse, op.getBranchWeightsAttr(),
        *convertedTrueBlock, *convertedFalseBlock);
    return success();
  }
};

/// Convert the destination block signatures (if necessary) and change the
/// operands of the switch op.
struct SwitchOpConversion : public OpConversionPattern<cf::SwitchOp> {
  using OpConversionPattern<cf::SwitchOp>::OpConversionPattern;

  LogicalResult
  matchAndRewrite(cf::SwitchOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const override {
    // Get or convert default block.
    FailureOr<Block *> convertedDefaultBlock = getConvertedBlock(
        rewriter, getTypeConverter(), op, op.getDefaultDestination(),
        TypeRange(adaptor.getDefaultOperands()));
    if (failed(convertedDefaultBlock))
      return failure();

    // Get or convert all case blocks.
    SmallVector<Block *> caseDestinations;
    SmallVector<ValueRange> caseOperands = adaptor.getCaseOperands();
    for (auto it : toolchain::enumerate(op.getCaseDestinations())) {
      Block *b = it.value();
      FailureOr<Block *> convertedBlock =
          getConvertedBlock(rewriter, getTypeConverter(), op, b,
                            TypeRange(caseOperands[it.index()]));
      if (failed(convertedBlock))
        return failure();
      caseDestinations.push_back(*convertedBlock);
    }

    rewriter.replaceOpWithNewOp<cf::SwitchOp>(
        op, adaptor.getFlag(), *convertedDefaultBlock,
        adaptor.getDefaultOperands(), adaptor.getCaseValuesAttr(),
        caseDestinations, caseOperands);
    return success();
  }
};

} // namespace

void mlir::cf::populateCFStructuralTypeConversions(
    const TypeConverter &typeConverter, RewritePatternSet &patterns,
    PatternBenefit benefit) {
  patterns.add<BranchOpConversion, CondBranchOpConversion, SwitchOpConversion>(
      typeConverter, patterns.getContext(), benefit);
}

void mlir::cf::populateCFStructuralTypeConversionTarget(
    const TypeConverter &typeConverter, ConversionTarget &target) {
  target.addDynamicallyLegalOp<cf::BranchOp, cf::CondBranchOp, cf::SwitchOp>(
      [&](Operation *op) { return typeConverter.isLegal(op->getOperands()); });
}

void mlir::cf::populateCFStructuralTypeConversionsAndLegality(
    const TypeConverter &typeConverter, RewritePatternSet &patterns,
    ConversionTarget &target, PatternBenefit benefit) {
  populateCFStructuralTypeConversions(typeConverter, patterns, benefit);
  populateCFStructuralTypeConversionTarget(typeConverter, target);
}
