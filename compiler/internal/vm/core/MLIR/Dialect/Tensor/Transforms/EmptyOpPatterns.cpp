//===- EmptyOpPatterns.cpp - Patterns related to tensor.empty folding ----===//
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
#include "mlir/Dialect/Tensor/IR/Tensor.h"
#include "mlir/Dialect/Tensor/Transforms/Transforms.h"
#include "mlir/IR/PatternMatch.h"

using namespace mlir;
using namespace mlir::tensor;

namespace {

template <typename ReshapeOp>
struct FoldEmptyTensorWithReshapeOp : public OpRewritePattern<ReshapeOp> {
  FoldEmptyTensorWithReshapeOp(MLIRContext *ctx, PatternBenefit benefit = 1,
                               bool foldSingleUseOnly = false)
      : OpRewritePattern<ReshapeOp>(ctx, benefit),
        foldSingleUseOnly(foldSingleUseOnly) {}

  LogicalResult matchAndRewrite(ReshapeOp reshapeOp,
                                PatternRewriter &rewriter) const override {
    // Check for tensor.empty source.
    auto emptyOp = reshapeOp.getSrc().template getDefiningOp<EmptyOp>();
    if (!emptyOp)
      return failure();

    // Check for single use.
    if (foldSingleUseOnly && !toolchain::hasSingleElement(emptyOp->getUses()))
      return failure();

    // Reify result shape.
    Location loc = reshapeOp.getLoc();
    ReifiedRankedShapedTypeDims resultShapes;
    if (failed(reifyResultShapes(rewriter, reshapeOp, resultShapes)) ||
        !toolchain::hasSingleElement(resultShapes))
      return failure();

    // Create new tensor.empty op.
    // TODO: Do not drop tensor type encoding.
    Value emptyTensor =
        EmptyOp::create(rewriter, loc, resultShapes[0],
                        reshapeOp.getResultType().getElementType());
    if (emptyTensor.getType() != reshapeOp.getResultType()) {
      rewriter.replaceOpWithNewOp<tensor::CastOp>(
          reshapeOp, reshapeOp.getResultType(), emptyTensor);
    } else {
      rewriter.replaceOp(reshapeOp, emptyTensor);
    }
    return success();
  }

private:
  bool foldSingleUseOnly = false;
};

/// tensor.empty does not define any tensor contents, so a slice of a
/// tensor.empty can be folded to a smaller tensor.empty.
struct FoldEmptyTensorWithExtractSliceOp
    : public OpRewritePattern<ExtractSliceOp> {
  FoldEmptyTensorWithExtractSliceOp(MLIRContext *ctx,
                                    PatternBenefit benefit = 1,
                                    bool foldSingleUseOnly = false)
      : OpRewritePattern<ExtractSliceOp>(ctx, benefit),
        foldSingleUseOnly(foldSingleUseOnly) {}

  LogicalResult matchAndRewrite(ExtractSliceOp sliceOp,
                                PatternRewriter &rewriter) const override {
    // Check for tensor.empty source.
    auto emptyOp = sliceOp.getSource().template getDefiningOp<EmptyOp>();
    if (!emptyOp)
      return failure();

    // Check for single use.
    if (foldSingleUseOnly && !toolchain::hasSingleElement(emptyOp->getUses()))
      return failure();

    // Create new tensor.empty op. tensor.extract_slice may be rank-reducing;
    // its dynamic sizes must be preserved as well as its result type.
    auto tensorType = RankedTensorType::get(sliceOp.getType().getShape(),
                                            sliceOp.getType().getElementType(),
                                            sliceOp.getType().getEncoding());
    rewriter.replaceOpWithNewOp<EmptyOp>(sliceOp, tensorType,
                                         sliceOp.getSizes());
    return success();
  }

private:
  bool foldSingleUseOnly = false;
};

// Fold concat operation where all the operands are empty.
struct FoldConcatsOfEmpty : public OpRewritePattern<ConcatOp> {
  using OpRewritePattern<ConcatOp>::OpRewritePattern;

  LogicalResult matchAndRewrite(tensor::ConcatOp concatOp,
                                PatternRewriter &rewriter) const override {
    auto concatOperands = concatOp.getInputs();
    if (concatOperands.empty()) {
      return failure();
    }
    auto firstEmptyOp = concatOperands.front().getDefiningOp<tensor::EmptyOp>();
    if (!firstEmptyOp) {
      return failure();
    }
    auto isDefinedByEmptyOp = [](Value v) -> bool {
      return v.getDefiningOp<tensor::EmptyOp>();
    };
    if (!toolchain::all_of(concatOperands.drop_front(), isDefinedByEmptyOp)) {
      return rewriter.notifyMatchFailure(
          concatOp, "not all operands are defined by an empty op");
    }
    SmallVector<SmallVector<OpFoldResult>> resultShape;
    if (failed(concatOp.reifyResultShapes(rewriter, resultShape))) {
      return rewriter.notifyMatchFailure(concatOp,
                                         "failed to get result shape");
    }
    rewriter.replaceOpWithNewOp<tensor::EmptyOp>(
        concatOp, resultShape[0], concatOp.getResultType().getElementType());
    return success();
  }
};

} // namespace

void mlir::tensor::populateFoldTensorEmptyPatterns(RewritePatternSet &patterns,
                                                   bool foldSingleUseOnly) {
  patterns.add<FoldEmptyTensorWithExtractSliceOp,
               FoldEmptyTensorWithReshapeOp<tensor::ExpandShapeOp>,
               FoldEmptyTensorWithReshapeOp<tensor::CollapseShapeOp>>(
      patterns.getContext(), /*benefit=*/1, foldSingleUseOnly);
  patterns.add<FoldConcatsOfEmpty>(patterns.getContext(),
                                   /*benefit=*/1);
}
