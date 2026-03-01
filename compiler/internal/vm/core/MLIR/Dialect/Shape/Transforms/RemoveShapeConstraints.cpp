//===-- RemoveShapeConstraints.cpp - Remove Shape Cstr and Assuming Ops ---===//
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

#include "mlir/Dialect/Shape/Transforms/Passes.h"

#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/Shape/IR/Shape.h"
#include "mlir/Transforms/DialectConversion.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"

namespace mlir {
#define GEN_PASS_DEF_REMOVESHAPECONSTRAINTSPASS
#include "mlir/Dialect/Shape/Transforms/Passes.h.inc"
} // namespace mlir

using namespace mlir;

namespace {
/// Removal patterns.
class RemoveCstrBroadcastableOp
    : public OpRewritePattern<shape::CstrBroadcastableOp> {
public:
  using OpRewritePattern::OpRewritePattern;

  LogicalResult matchAndRewrite(shape::CstrBroadcastableOp op,
                                PatternRewriter &rewriter) const override {
    rewriter.replaceOpWithNewOp<shape::ConstWitnessOp>(op.getOperation(), true);
    return success();
  }
};

class RemoveCstrEqOp : public OpRewritePattern<shape::CstrEqOp> {
public:
  using OpRewritePattern::OpRewritePattern;

  LogicalResult matchAndRewrite(shape::CstrEqOp op,
                                PatternRewriter &rewriter) const override {
    rewriter.replaceOpWithNewOp<shape::ConstWitnessOp>(op.getOperation(), true);
    return success();
  }
};

/// Removal pass.
class RemoveShapeConstraintsPass
    : public impl::RemoveShapeConstraintsPassBase<RemoveShapeConstraintsPass> {

  void runOnOperation() override {
    MLIRContext &ctx = getContext();

    RewritePatternSet patterns(&ctx);
    populateRemoveShapeConstraintsPatterns(patterns);

    (void)applyPatternsGreedily(getOperation(), std::move(patterns));
  }
};

} // namespace

void mlir::populateRemoveShapeConstraintsPatterns(RewritePatternSet &patterns) {
  patterns.add<RemoveCstrBroadcastableOp, RemoveCstrEqOp>(
      patterns.getContext());
}
