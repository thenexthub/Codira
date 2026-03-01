//===- ExpandDivs.cpp - Expansion patterns for MemRef operations ----------===//
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

#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Arith/Transforms/Passes.h"
#include "mlir/Dialect/MemRef/IR/MemRef.h"
#include "mlir/Dialect/MemRef/Transforms/Transforms.h"
#include "mlir/IR/TypeUtilities.h"
#include "mlir/Transforms/DialectConversion.h"

namespace mlir {
namespace memref {
#define GEN_PASS_DEF_EXPANDOPSPASS
#include "mlir/Dialect/MemRef/Transforms/Passes.h.inc"
} // namespace memref
} // namespace mlir

using namespace mlir;

namespace {

/// Converts `memref.reshape` that has a target shape of a statically-known
/// size to `memref.reinterpret_cast`.
struct MemRefReshapeOpConverter : public OpRewritePattern<memref::ReshapeOp> {
public:
  using OpRewritePattern::OpRewritePattern;

  LogicalResult matchAndRewrite(memref::ReshapeOp op,
                                PatternRewriter &rewriter) const final {
    auto shapeType = cast<MemRefType>(op.getShape().getType());
    if (!shapeType.hasStaticShape())
      return failure();

    int64_t rank = cast<MemRefType>(shapeType).getDimSize(0);
    SmallVector<OpFoldResult, 4> sizes, strides;
    sizes.resize(rank);
    strides.resize(rank);

    Location loc = op.getLoc();
    Value stride = nullptr;
    int64_t staticStride = 1;
    for (int i = rank - 1; i >= 0; --i) {
      Value size;
      // Load dynamic sizes from the shape input, use constants for static dims.
      if (op.getType().isDynamicDim(i)) {
        Value index = arith::ConstantIndexOp::create(rewriter, loc, i);
        size = memref::LoadOp::create(rewriter, loc, op.getShape(), index);
        if (!isa<IndexType>(size.getType()))
          size = arith::IndexCastOp::create(rewriter, loc,
                                            rewriter.getIndexType(), size);
        sizes[i] = size;
      } else {
        auto sizeAttr = rewriter.getIndexAttr(op.getType().getDimSize(i));
        size = arith::ConstantOp::create(rewriter, loc, sizeAttr);
        sizes[i] = sizeAttr;
      }
      if (stride)
        strides[i] = stride;
      else
        strides[i] = rewriter.getIndexAttr(staticStride);

      if (i > 0) {
        if (stride) {
          stride = arith::MulIOp::create(rewriter, loc, stride, size);
        } else if (op.getType().isDynamicDim(i)) {
          stride = arith::MulIOp::create(
              rewriter, loc,
              arith::ConstantIndexOp::create(rewriter, loc, staticStride),
              size);
        } else {
          staticStride *= op.getType().getDimSize(i);
        }
      }
    }
    rewriter.replaceOpWithNewOp<memref::ReinterpretCastOp>(
        op, op.getType(), op.getSource(), /*offset=*/rewriter.getIndexAttr(0),
        sizes, strides);
    return success();
  }
};

struct ExpandOpsPass : public memref::impl::ExpandOpsPassBase<ExpandOpsPass> {
  void runOnOperation() override {
    MLIRContext &ctx = getContext();

    RewritePatternSet patterns(&ctx);
    memref::populateExpandOpsPatterns(patterns);
    ConversionTarget target(ctx);

    target.addLegalDialect<arith::ArithDialect, memref::MemRefDialect>();
    target.addDynamicallyLegalOp<memref::ReshapeOp>([](memref::ReshapeOp op) {
      return !cast<MemRefType>(op.getShape().getType()).hasStaticShape();
    });
    if (failed(applyPartialConversion(getOperation(), target,
                                      std::move(patterns))))
      signalPassFailure();
  }
};

} // namespace

void mlir::memref::populateExpandOpsPatterns(RewritePatternSet &patterns) {
  patterns.add<MemRefReshapeOpConverter>(patterns.getContext());
}
