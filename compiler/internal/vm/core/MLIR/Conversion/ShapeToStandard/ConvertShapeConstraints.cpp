//===- ConvertShapeConstraints.cpp - Conversion of shape constraints ------===//
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

#include "mlir/Conversion/ShapeToStandard/ShapeToStandard.h"

#include "mlir/Dialect/ControlFlow/IR/ControlFlowOps.h"
#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/Dialect/Shape/IR/Shape.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"

namespace mlir {
#define GEN_PASS_DEF_CONVERTSHAPECONSTRAINTSPASS
#include "mlir/Conversion/Passes.h.inc"
} // namespace mlir

using namespace mlir;

namespace {
#include "ShapeToStandard.cpp.inc"
} // namespace

namespace {
class ConvertCstrRequireOp : public OpRewritePattern<shape::CstrRequireOp> {
public:
  using OpRewritePattern::OpRewritePattern;
  LogicalResult matchAndRewrite(shape::CstrRequireOp op,
                                PatternRewriter &rewriter) const override {
    cf::AssertOp::create(rewriter, op.getLoc(), op.getPred(), op.getMsgAttr());
    rewriter.replaceOpWithNewOp<shape::ConstWitnessOp>(op, true);
    return success();
  }
};
} // namespace

void mlir::populateConvertShapeConstraintsConversionPatterns(
    RewritePatternSet &patterns) {
  patterns.add<CstrBroadcastableToRequire>(patterns.getContext());
  patterns.add<CstrEqToRequire>(patterns.getContext());
  patterns.add<ConvertCstrRequireOp>(patterns.getContext());
}

namespace {
// This pass eliminates shape constraints from the program, converting them to
// eager (side-effecting) error handling code. After eager error handling code
// is emitted, witnesses are satisfied, so they are replace with
// `shape.const_witness true`.
class ConvertShapeConstraints
    : public impl::ConvertShapeConstraintsPassBase<ConvertShapeConstraints> {
  void runOnOperation() override {
    auto *func = getOperation();
    auto *context = &getContext();

    RewritePatternSet patterns(context);
    populateConvertShapeConstraintsConversionPatterns(patterns);

    if (failed(applyPatternsGreedily(func, std::move(patterns))))
      return signalPassFailure();
  }
};
} // namespace
