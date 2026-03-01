//===- FoldMemRefAliasOps.cpp - Fold memref alias ops for affine ops ------===//
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
// This pass contains affine-specif versions of the folding patterns for
// memref.expand_shape, memref.collapse_shape, and memref.subview, since
// those all need affine-specific handling that won't fit a general interface.
//
//===----------------------------------------------------------------------===//

#include "mlir/Dialect/Affine/IR/AffineOps.h"
#include "mlir/Dialect/Affine/Transforms/Passes.h"
#include "mlir/Dialect/Affine/ViewLikeInterfaceUtils.h"
#include "mlir/Dialect/Arith/Utils/Utils.h"
#include "mlir/Dialect/MemRef/IR/MemRef.h"
#include "mlir/Dialect/MemRef/Utils/MemRefUtils.h"
#include "mlir/IR/AffineMap.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"

namespace mlir {
namespace affine {
#define GEN_PASS_DEF_AFFINEFOLDMEMREFALIASOPS
#include "mlir/Dialect/Affine/Transforms/Passes.h.inc"
} // namespace affine
} // namespace mlir

using namespace mlir;
using namespace mlir::affine;

//===----------------------------------------------------------------------===//
// Utility functions
//===----------------------------------------------------------------------===//

/// Given an AffineMap and a list of indices, apply the map to get the
/// underlying indices (expanding the affine map).
static void expandToUnderlyingIndices(AffineMap affineMap, ValueRange indices,
                                      Location loc, PatternRewriter &rewriter,
                                      SmallVectorImpl<Value> &result) {
  SmallVector<OpFoldResult> indicesOfr(
      toolchain::map_to_vector(indices, [](Value v) -> OpFoldResult { return v; }));
  for (unsigned i : toolchain::seq(0u, affineMap.getNumResults())) {
    OpFoldResult ofr = affine::makeComposedFoldedAffineApply(
        rewriter, loc, affineMap.getSubMap({i}), indicesOfr);
    result.push_back(getValueOrCreateConstantIndexOp(rewriter, loc, ofr));
  }
}

//===----------------------------------------------------------------------===//
// Patterns
//===----------------------------------------------------------------------===//

namespace {

struct AffineLoadOpOfSubViewOpFolder final : OpRewritePattern<AffineLoadOp> {
  using Base::Base;

  LogicalResult matchAndRewrite(AffineLoadOp loadOp,
                                PatternRewriter &rewriter) const override {
    auto subViewOp = loadOp.getMemref().getDefiningOp<memref::SubViewOp>();

    if (!subViewOp)
      return rewriter.notifyMatchFailure(loadOp, "not a subview producer");

    SmallVector<Value> indices;
    expandToUnderlyingIndices(loadOp.getAffineMap(), loadOp.getIndices(),
                              loadOp.getLoc(), rewriter, indices);

    SmallVector<Value> sourceIndices;
    affine::resolveIndicesIntoOpWithOffsetsAndStrides(
        rewriter, loadOp.getLoc(), subViewOp.getMixedOffsets(),
        subViewOp.getMixedStrides(), subViewOp.getDroppedDims(), indices,
        sourceIndices);

    rewriter.replaceOpWithNewOp<AffineLoadOp>(loadOp, subViewOp.getSource(),
                                              sourceIndices);
    return success();
  }
};

struct AffineLoadOpOfExpandShapeOpFolder final
    : OpRewritePattern<AffineLoadOp> {
  using Base::Base;

  LogicalResult matchAndRewrite(AffineLoadOp loadOp,
                                PatternRewriter &rewriter) const override {
    auto expandShapeOp =
        loadOp.getMemref().getDefiningOp<memref::ExpandShapeOp>();

    if (!expandShapeOp)
      return failure();

    SmallVector<Value> indices;
    expandToUnderlyingIndices(loadOp.getAffineMap(), loadOp.getIndices(),
                              loadOp.getLoc(), rewriter, indices);

    SmallVector<Value> sourceIndices;
    // affine.load guarantees that indexes start inbounds, which impacts if our
    // linearization is `disjoint`.
    memref::resolveSourceIndicesExpandShape(
        loadOp.getLoc(), rewriter, expandShapeOp, indices, sourceIndices,
        /*startsInbounds=*/true);

    rewriter.replaceOpWithNewOp<AffineLoadOp>(
        loadOp, expandShapeOp.getViewSource(), sourceIndices);
    return success();
  }
};

struct AffineLoadOpOfCollapseShapeOpFolder final
    : OpRewritePattern<AffineLoadOp> {
  using Base::Base;

  LogicalResult matchAndRewrite(AffineLoadOp loadOp,
                                PatternRewriter &rewriter) const override {
    auto collapseShapeOp =
        loadOp.getMemref().getDefiningOp<memref::CollapseShapeOp>();

    if (!collapseShapeOp)
      return failure();

    SmallVector<Value> indices;
    expandToUnderlyingIndices(loadOp.getAffineMap(), loadOp.getIndices(),
                              loadOp.getLoc(), rewriter, indices);

    SmallVector<Value> sourceIndices;
    memref::resolveSourceIndicesCollapseShape(
        loadOp.getLoc(), rewriter, collapseShapeOp, indices, sourceIndices);

    rewriter.replaceOpWithNewOp<AffineLoadOp>(
        loadOp, collapseShapeOp.getViewSource(), sourceIndices);
    return success();
  }
};

struct AffineStoreOpOfSubViewOpFolder final : OpRewritePattern<AffineStoreOp> {
  using Base::Base;

  LogicalResult matchAndRewrite(AffineStoreOp storeOp,
                                PatternRewriter &rewriter) const override {
    auto subViewOp = storeOp.getMemref().getDefiningOp<memref::SubViewOp>();

    if (!subViewOp)
      return rewriter.notifyMatchFailure(storeOp, "not a subview producer");

    // For affine ops, we need to apply the map to get the "actual" indices.
    SmallVector<Value> indices;
    expandToUnderlyingIndices(storeOp.getAffineMap(), storeOp.getIndices(),
                              storeOp.getLoc(), rewriter, indices);

    SmallVector<Value> sourceIndices;
    affine::resolveIndicesIntoOpWithOffsetsAndStrides(
        rewriter, storeOp.getLoc(), subViewOp.getMixedOffsets(),
        subViewOp.getMixedStrides(), subViewOp.getDroppedDims(), indices,
        sourceIndices);

    rewriter.replaceOpWithNewOp<AffineStoreOp>(
        storeOp, storeOp.getValue(), subViewOp.getSource(), sourceIndices);
    return success();
  }
};

struct AffineStoreOpOfExpandShapeOpFolder final
    : OpRewritePattern<AffineStoreOp> {
  using Base::Base;

  LogicalResult matchAndRewrite(AffineStoreOp storeOp,
                                PatternRewriter &rewriter) const override {
    auto expandShapeOp =
        storeOp.getMemref().getDefiningOp<memref::ExpandShapeOp>();

    if (!expandShapeOp)
      return failure();

    SmallVector<Value> indices;
    expandToUnderlyingIndices(storeOp.getAffineMap(), storeOp.getIndices(),
                              storeOp.getLoc(), rewriter, indices);

    SmallVector<Value> sourceIndices;
    // affine.store guarantees that indexes start inbounds, which impacts if our
    // linearization is `disjoint`.
    memref::resolveSourceIndicesExpandShape(
        storeOp.getLoc(), rewriter, expandShapeOp, indices, sourceIndices,
        /*startsInbounds=*/true);

    rewriter.replaceOpWithNewOp<AffineStoreOp>(
        storeOp, storeOp.getValueToStore(), expandShapeOp.getViewSource(),
        sourceIndices);
    return success();
  }
};

struct AffineStoreOpOfCollapseShapeOpFolder final
    : OpRewritePattern<AffineStoreOp> {
  using Base::Base;

  LogicalResult matchAndRewrite(AffineStoreOp storeOp,
                                PatternRewriter &rewriter) const override {
    auto collapseShapeOp =
        storeOp.getMemref().getDefiningOp<memref::CollapseShapeOp>();

    if (!collapseShapeOp)
      return failure();

    // For affine ops, we need to apply the map to get the "actual" indices.
    SmallVector<Value> indices;
    expandToUnderlyingIndices(storeOp.getAffineMap(), storeOp.getIndices(),
                              storeOp.getLoc(), rewriter, indices);

    SmallVector<Value> sourceIndices;
    memref::resolveSourceIndicesCollapseShape(
        storeOp.getLoc(), rewriter, collapseShapeOp, indices, sourceIndices);

    rewriter.replaceOpWithNewOp<AffineStoreOp>(
        storeOp, storeOp.getValueToStore(), collapseShapeOp.getViewSource(),
        sourceIndices);
    return success();
  }
};

} // namespace

void affine::populateAffineFoldMemRefAliasOpPatterns(
    RewritePatternSet &patterns) {
  patterns
      .add<AffineLoadOpOfSubViewOpFolder, AffineLoadOpOfExpandShapeOpFolder,
           AffineLoadOpOfCollapseShapeOpFolder, AffineStoreOpOfSubViewOpFolder,
           AffineStoreOpOfExpandShapeOpFolder,
           AffineStoreOpOfCollapseShapeOpFolder>(patterns.getContext());
}

//===----------------------------------------------------------------------===//
// Pass registration
//===----------------------------------------------------------------------===//

namespace {

struct AffineFoldMemRefAliasOpsPass final
    : public affine::impl::AffineFoldMemRefAliasOpsBase<
          AffineFoldMemRefAliasOpsPass> {
  void runOnOperation() override;
};

} // namespace

void AffineFoldMemRefAliasOpsPass::runOnOperation() {
  RewritePatternSet patterns(&getContext());
  affine::populateAffineFoldMemRefAliasOpPatterns(patterns);
  (void)applyPatternsGreedily(getOperation(), std::move(patterns));
}
