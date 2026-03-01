//===- GlobalIdRewriter.cpp - Implementation of GlobalId rewriting  -------===//
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
// This file implements in-dialect rewriting of the global_id op for archs
// where global_id.x = threadId.x + blockId.x * blockDim.x
//
//===----------------------------------------------------------------------===//

#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/GPU/IR/GPUDialect.h"
#include "mlir/Dialect/GPU/Transforms/Passes.h"
#include "mlir/IR/PatternMatch.h"

using namespace mlir;

namespace {
struct GpuGlobalIdRewriter : public OpRewritePattern<gpu::GlobalIdOp> {
  using OpRewritePattern<gpu::GlobalIdOp>::OpRewritePattern;

  LogicalResult matchAndRewrite(gpu::GlobalIdOp op,
                                PatternRewriter &rewriter) const override {
    Location loc = op.getLoc();
    auto dim = op.getDimension();
    Value blockId = gpu::BlockIdOp::create(rewriter, loc, dim);
    Value blockDim = gpu::BlockDimOp::create(rewriter, loc, dim);
    auto indexType = rewriter.getIndexType();
    // Compute blockId.x * blockDim.x
    Value tmp =
        arith::MulIOp::create(rewriter, loc, indexType, blockId, blockDim);
    Value threadId = gpu::ThreadIdOp::create(rewriter, loc, dim);
    // Compute threadId.x + blockId.x * blockDim.x
    rewriter.replaceOpWithNewOp<arith::AddIOp>(op, indexType, threadId, tmp);
    return success();
  }
};
} // namespace

void mlir::populateGpuGlobalIdPatterns(RewritePatternSet &patterns) {
  patterns.add<GpuGlobalIdRewriter>(patterns.getContext());
}
