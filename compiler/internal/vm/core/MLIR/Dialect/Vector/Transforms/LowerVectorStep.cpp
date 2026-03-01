//===- LowerVectorStep.cpp - Lower 'vector.step' operation ----------------===//
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
// This file implements target-independent rewrites and utilities to lower the
// 'vector.step' operation.
//
//===----------------------------------------------------------------------===//

#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Vector/IR/VectorOps.h"
#include "mlir/Dialect/Vector/Transforms/LoweringPatterns.h"
#include "mlir/IR/PatternMatch.h"

#define DEBUG_TYPE "vector-step-lowering"

using namespace mlir;
using namespace mlir::vector;

namespace {

struct StepToArithConstantOpRewrite final : OpRewritePattern<vector::StepOp> {
  using Base::Base;

  LogicalResult matchAndRewrite(vector::StepOp stepOp,
                                PatternRewriter &rewriter) const override {
    auto resultType = cast<VectorType>(stepOp.getType());
    if (resultType.isScalable()) {
      return failure();
    }
    int64_t elementCount = resultType.getNumElements();
    SmallVector<APInt> indices =
        toolchain::map_to_vector(toolchain::seq(elementCount),
                            [](int64_t i) { return APInt(/*width=*/64, i); });
    rewriter.replaceOpWithNewOp<arith::ConstantOp>(
        stepOp, DenseElementsAttr::get(resultType, indices));
    return success();
  }
};
} // namespace

void mlir::vector::populateVectorStepLoweringPatterns(
    RewritePatternSet &patterns, PatternBenefit benefit) {
  patterns.add<StepToArithConstantOpRewrite>(patterns.getContext(), benefit);
}
