//===- SwapExtractSliceWithFillPatterns.cpp -------------------------------===//
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

#include "mlir/Dialect/Linalg/Transforms/Transforms.h"
#include "mlir/IR/PatternMatch.h"

using namespace mlir;
using namespace mlir::linalg;

/// swaps:
///      `tensor.extract_slice(linalg.fill(%cst, %init))`
/// with:
///      `linalg.fill(%cst, tensor.extract_slice(%init))`
///
/// when the linalg.fill op have no other users.
/// This helps to reduce the fill footprint.
struct SwapExtractSliceOfFill final
    : public OpRewritePattern<tensor::ExtractSliceOp> {
  using OpRewritePattern::OpRewritePattern;

  LogicalResult matchAndRewrite(tensor::ExtractSliceOp extractOp,
                                PatternRewriter &rewriter) const override {
    auto fillOp = extractOp.getSource().getDefiningOp<FillOp>();
    if (!fillOp || !fillOp->hasOneUse())
      return failure();

    auto newExtractOp = tensor::ExtractSliceOp::create(
        rewriter, extractOp.getLoc(), extractOp.getType(),
        fillOp.getOutputs()[0], extractOp.getMixedOffsets(),
        extractOp.getMixedSizes(), extractOp.getMixedStrides());
    rewriter.replaceOpWithNewOp<FillOp>(extractOp, fillOp.getInputs(),
                                        ValueRange{newExtractOp.getResult()});
    return success();
  }
};

void mlir::linalg::populateSwapExtractSliceWithFillPatterns(
    RewritePatternSet &patterns) {
  patterns.add<SwapExtractSliceOfFill>(patterns.getContext());
}
