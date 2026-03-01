//===- XeGPUFoldAliasOps.cpp - XeGPU alias ops folders ----------*- C++ -*-===//
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

#include "mlir/Dialect/XeGPU/Transforms/Passes.h"

#include "mlir/Dialect/Affine/ViewLikeInterfaceUtils.h"
#include "mlir/Dialect/MemRef/IR/MemRef.h"
#include "mlir/Dialect/XeGPU/IR/XeGPU.h"
#include "mlir/Dialect/XeGPU/Transforms/Transforms.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"

namespace mlir {
namespace xegpu {
#define GEN_PASS_DEF_XEGPUFOLDALIASOPS
#include "mlir/Dialect/XeGPU/Transforms/Passes.h.inc"
} // namespace xegpu
} // namespace mlir

#define DEBUG_TYPE "xegpu-fold-alias-ops"
#define DBGS() (toolchain::dbgs() << "[" DEBUG_TYPE "]: ")

using namespace mlir;

namespace {
/// Merges subview operation with xegpu.create_nd_tdesc operation.
class XegpuCreateNdDescOpSubViewOpFolder final
    : public OpRewritePattern<xegpu::CreateNdDescOp> {
public:
  using OpRewritePattern<xegpu::CreateNdDescOp>::OpRewritePattern;

  LogicalResult matchAndRewrite(xegpu::CreateNdDescOp descOp,
                                PatternRewriter &rewriter) const override;
};
} // namespace

LogicalResult XegpuCreateNdDescOpSubViewOpFolder::matchAndRewrite(
    xegpu::CreateNdDescOp descOp, PatternRewriter &rewriter) const {
  auto subViewOp = descOp.getSource().getDefiningOp<memref::SubViewOp>();

  if (!subViewOp)
    return rewriter.notifyMatchFailure(descOp, "not a subview producer");
  if (!subViewOp.hasUnitStride())
    return rewriter.notifyMatchFailure(descOp, "requires unit strides");

  SmallVector<Value> resolvedOffsets;
  affine::resolveIndicesIntoOpWithOffsetsAndStrides(
      rewriter, descOp.getLoc(), subViewOp.getMixedOffsets(),
      subViewOp.getMixedStrides(), subViewOp.getDroppedDims(),
      descOp.getMixedOffsets(), resolvedOffsets);

  rewriter.replaceOpWithNewOp<xegpu::CreateNdDescOp>(
      descOp, descOp.getTensorDesc().getType(), subViewOp.getSource(),
      getAsOpFoldResult(resolvedOffsets));

  return success();
}

void xegpu::populateXeGPUFoldAliasOpsPatterns(RewritePatternSet &patterns) {
  patterns.add<XegpuCreateNdDescOpSubViewOpFolder>(patterns.getContext());
}

namespace {

struct XeGPUFoldAliasOpsPass final
    : public xegpu::impl::XeGPUFoldAliasOpsBase<XeGPUFoldAliasOpsPass> {
  void runOnOperation() override;
};

} // namespace

void XeGPUFoldAliasOpsPass::runOnOperation() {
  RewritePatternSet patterns(&getContext());
  xegpu::populateXeGPUFoldAliasOpsPatterns(patterns);
  (void)applyPatternsGreedily(getOperation(), std::move(patterns));
}
