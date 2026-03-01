//===- UpliftToFMA.cpp - Arith to FMA uplifting ---------------------------===//
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
// This file implements uplifting from arith ops to math.fma.
//
//===----------------------------------------------------------------------===//

#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Math/IR/Math.h"
#include "mlir/Dialect/Math/Transforms/Passes.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"

namespace mlir::math {
#define GEN_PASS_DEF_MATHUPLIFTTOFMA
#include "mlir/Dialect/Math/Transforms/Passes.h.inc"
} // namespace mlir::math

using namespace mlir;

template <typename Op>
static bool isValidForFMA(Op op) {
  return static_cast<bool>(op.getFastmath() & arith::FastMathFlags::contract);
}

namespace {

struct UpliftFma final : OpRewritePattern<arith::AddFOp> {
  using OpRewritePattern::OpRewritePattern;

  LogicalResult matchAndRewrite(arith::AddFOp op,
                                PatternRewriter &rewriter) const override {
    if (!isValidForFMA(op))
      return rewriter.notifyMatchFailure(op, "addf op is not suitable for fma");

    Value c;
    arith::MulFOp ab;
    if ((ab = op.getLhs().getDefiningOp<arith::MulFOp>())) {
      c = op.getRhs();
    } else if ((ab = op.getRhs().getDefiningOp<arith::MulFOp>())) {
      c = op.getLhs();
    } else {
      return rewriter.notifyMatchFailure(op, "no mulf op");
    }

    if (!isValidForFMA(ab))
      return rewriter.notifyMatchFailure(ab, "mulf op is not suitable for fma");

    Value a = ab.getLhs();
    Value b = ab.getRhs();
    arith::FastMathFlags fmf = op.getFastmath() & ab.getFastmath();
    rewriter.replaceOpWithNewOp<math::FmaOp>(op, a, b, c, fmf);
    return success();
  }
};

struct MathUpliftToFMA final
    : math::impl::MathUpliftToFMABase<MathUpliftToFMA> {
  using MathUpliftToFMABase::MathUpliftToFMABase;

  void runOnOperation() override {
    RewritePatternSet patterns(&getContext());
    populateUpliftToFMAPatterns(patterns);
    if (failed(applyPatternsGreedily(getOperation(), std::move(patterns))))
      return signalPassFailure();
  }
};

} // namespace

void mlir::populateUpliftToFMAPatterns(RewritePatternSet &patterns) {
  patterns.insert<UpliftFma>(patterns.getContext());
}
