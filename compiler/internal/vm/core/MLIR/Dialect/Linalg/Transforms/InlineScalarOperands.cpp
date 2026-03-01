//===- InlineScalarOperands.cpp - Pass to inline scalar operands =============//
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
// This file implements patterns/pass to inline scalar operands into a generic
// operation. A scalar operand is an operand whose indexing map has a constant
// rhs.
//
//===----------------------------------------------------------------------===//

#include "mlir/Dialect/Linalg/Passes.h"

#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/Linalg/IR/Linalg.h"
#include "mlir/Dialect/Linalg/Transforms/Transforms.h"
#include "mlir/IR/AffineExpr.h"
#include "mlir/IR/AffineMap.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"

namespace mlir {
#define GEN_PASS_DEF_LINALGINLINESCALAROPERANDSPASS
#include "mlir/Dialect/Linalg/Passes.h.inc"
} // namespace mlir

using namespace mlir;
using namespace mlir::linalg;

namespace {
struct InlineScalarOperands : public OpRewritePattern<GenericOp> {
  using OpRewritePattern<GenericOp>::OpRewritePattern;
  LogicalResult matchAndRewrite(GenericOp genericOp,
                                PatternRewriter &rewriter) const override {
    if (!genericOp.hasPureTensorSemantics())
      return failure();

    SmallVector<size_t> scalarOperands;
    SmallVector<AffineMap> newIndexingMaps;
    SmallVector<Value> newOperands;
    for (OpOperand *opOperand : genericOp.getDpsInputOperands()) {
      AffineMap map = genericOp.getMatchingIndexingMap(opOperand);
      if (genericOp.isDpsInput(opOperand) && map.isConstant()) {
        scalarOperands.emplace_back(opOperand->getOperandNumber());
      } else {
        newIndexingMaps.emplace_back(map);
        newOperands.emplace_back(opOperand->get());
      }
    }

    if (scalarOperands.empty())
      return failure();

    for (OpOperand &opOperand : genericOp.getDpsInitsMutable())
      newIndexingMaps.emplace_back(
          genericOp.getMatchingIndexingMap(&opOperand));

    Location loc = genericOp->getLoc();
    SmallVector<Value> outputOperands = genericOp.getOutputs();
    auto newOp = GenericOp::create(rewriter, loc, genericOp->getResultTypes(),
                                   newOperands, outputOperands, newIndexingMaps,
                                   genericOp.getIteratorTypesArray());
    rewriter.cloneRegionBefore(genericOp.getRegion(), newOp.getRegion(),
                               newOp.getRegion().begin());

    Block *body = newOp.getBody();
    PatternRewriter::InsertionGuard guard(rewriter);
    rewriter.setInsertionPointToStart(body);

    for (auto idx : toolchain::reverse(scalarOperands)) {
      OpOperand *opOperand = genericOp.getDpsInputOperand(idx);
      AffineMap map = genericOp.getMatchingIndexingMap(opOperand);
      SmallVector<int64_t> indices = map.getConstantResults();
      SmallVector<Value> indicesValues;
      for (auto idx : indices)
        indicesValues.emplace_back(
            arith::ConstantIndexOp::create(rewriter, loc, idx));
      Value scalarValue = opOperand->get();
      if (isa<RankedTensorType>(scalarValue.getType())) {
        scalarValue = tensor::ExtractOp::create(rewriter, loc, scalarValue,
                                                indicesValues);
      }
      body->getArgument(idx).replaceAllUsesWith(scalarValue);
      body->eraseArgument(idx);
    }

    rewriter.replaceOp(genericOp, newOp->getResults());
    return success();
  }
};
} // namespace

/// Patterns that are used to inline constant operands into linalg generic
/// ops.
void mlir::linalg::populateInlineConstantOperandsPatterns(
    RewritePatternSet &patterns) {
  auto *context = patterns.getContext();
  patterns.add<InlineScalarOperands>(context);
}

namespace {
/// Pass that removes unit-extent dims within generic ops.
struct LinalgInlineScalarOperandsPass
    : public impl::LinalgInlineScalarOperandsPassBase<
          LinalgInlineScalarOperandsPass> {
  using impl::LinalgInlineScalarOperandsPassBase<
      LinalgInlineScalarOperandsPass>::LinalgInlineScalarOperandsPassBase;
  void runOnOperation() override {
    Operation *op = getOperation();
    MLIRContext &ctx = getContext();
    RewritePatternSet patterns(&ctx);
    populateInlineConstantOperandsPatterns(patterns);
    (void)applyPatternsGreedily(op, std::move(patterns));
  }
};
} // namespace
