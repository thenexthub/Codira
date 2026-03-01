//===- SincosFusion.cpp - Fuse sin/cos into sincos -----------------------===//
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

#include "mlir/Dialect/Math/IR/Math.h"
#include "mlir/Dialect/Math/Transforms/Passes.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"

using namespace mlir;
using namespace mlir::math;

namespace {

/// Fuse a math.sin and math.cos in the same block that use the same operand and
/// have identical fastmath flags into a single math.sincos.
struct SincosFusionPattern : OpRewritePattern<math::SinOp> {
  using Base::Base;

  LogicalResult matchAndRewrite(math::SinOp sinOp,
                                PatternRewriter &rewriter) const override {
    Value operand = sinOp.getOperand();
    mlir::arith::FastMathFlags sinFastMathFlags = sinOp.getFastmath();

    math::CosOp cosOp = nullptr;
    sinOp->getBlock()->walk([&](math::CosOp op) {
      if (op.getOperand() == operand && op.getFastmath() == sinFastMathFlags) {
        cosOp = op;
        return WalkResult::interrupt();
      }
      return WalkResult::advance();
    });

    if (!cosOp)
      return failure();

    Operation *firstOp = sinOp->isBeforeInBlock(cosOp) ? sinOp.getOperation()
                                                       : cosOp.getOperation();
    rewriter.setInsertionPoint(firstOp);

    Type elemType = sinOp.getType();
    auto sincos = math::SincosOp::create(rewriter, firstOp->getLoc(),
                                         TypeRange{elemType, elemType}, operand,
                                         sinOp.getFastmathAttr());

    rewriter.replaceOp(sinOp, sincos.getSin());
    rewriter.replaceOp(cosOp, sincos.getCos());
    return success();
  }
};

} // namespace

namespace mlir::math {
#define GEN_PASS_DEF_MATHSINCOSFUSIONPASS
#include "mlir/Dialect/Math/Transforms/Passes.h.inc"
} // namespace mlir::math

namespace {

struct MathSincosFusionPass final
    : math::impl::MathSincosFusionPassBase<MathSincosFusionPass> {
  using MathSincosFusionPassBase::MathSincosFusionPassBase;

  void runOnOperation() override {
    RewritePatternSet patterns(&getContext());
    patterns.add<SincosFusionPattern>(&getContext());

    GreedyRewriteConfig config;
    if (failed(
            applyPatternsGreedily(getOperation(), std::move(patterns), config)))
      return signalPassFailure();
  }
};

} // namespace
