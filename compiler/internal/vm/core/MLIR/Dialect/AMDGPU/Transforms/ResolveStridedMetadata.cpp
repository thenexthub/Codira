//===- ResolveStridedMetadata.cpp - AMDGPU expand_strided_metadata ------===//
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

#include "mlir/Dialect/AMDGPU/Transforms/Passes.h"

#include "mlir/Dialect/AMDGPU/IR/AMDGPUDialect.h"
#include "mlir/Dialect/MemRef/IR/MemRef.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"

namespace mlir::amdgpu {
#define GEN_PASS_DEF_AMDGPURESOLVESTRIDEDMETADATAPASS
#include "mlir/Dialect/AMDGPU/Transforms/Passes.h.inc"
} // namespace mlir::amdgpu

using namespace mlir;
using namespace mlir::amdgpu;

namespace {
struct AmdgpuResolveStridedMetadataPass
    : public amdgpu::impl::AmdgpuResolveStridedMetadataPassBase<
          AmdgpuResolveStridedMetadataPass> {
  void runOnOperation() override;
};

struct ExtractStridedMetadataOnFatRawBufferCastFolder final
    : public OpRewritePattern<memref::ExtractStridedMetadataOp> {
  using OpRewritePattern::OpRewritePattern;
  LogicalResult matchAndRewrite(memref::ExtractStridedMetadataOp metadataOp,
                                PatternRewriter &rewriter) const override {
    auto castOp = metadataOp.getSource().getDefiningOp<FatRawBufferCastOp>();
    if (!castOp)
      return rewriter.notifyMatchFailure(metadataOp,
                                         "not a fat raw buffer cast");
    Location loc = castOp.getLoc();
    auto sourceMetadata = memref::ExtractStridedMetadataOp::create(
        rewriter, loc, castOp.getSource());
    SmallVector<Value> results;
    if (metadataOp.getBaseBuffer().use_empty()) {
      results.push_back(nullptr);
    } else {
      auto baseBufferType =
          cast<MemRefType>(metadataOp.getBaseBuffer().getType());
      if (baseBufferType == castOp.getResult().getType()) {
        results.push_back(castOp.getResult());
      } else {
        results.push_back(memref::ReinterpretCastOp::create(
            rewriter, loc, baseBufferType, castOp.getResult(), /*offset=*/0,
            /*sizes=*/ArrayRef<int64_t>{}, /*strides=*/ArrayRef<int64_t>{}));
      }
    }
    if (castOp.getResetOffset())
      results.push_back(arith::ConstantIndexOp::create(rewriter, loc, 0));
    else
      results.push_back(sourceMetadata.getOffset());
    toolchain::append_range(results, sourceMetadata.getSizes());
    toolchain::append_range(results, sourceMetadata.getStrides());
    rewriter.replaceOp(metadataOp, results);
    return success();
  }
};
} // namespace

void mlir::amdgpu::populateAmdgpuResolveStridedMetadataPatterns(
    RewritePatternSet &patterns, PatternBenefit benefit) {
  patterns.add<ExtractStridedMetadataOnFatRawBufferCastFolder>(
      patterns.getContext(), benefit);
}

void AmdgpuResolveStridedMetadataPass::runOnOperation() {
  RewritePatternSet patterns(&getContext());
  populateAmdgpuResolveStridedMetadataPatterns(patterns);
  if (failed(applyPatternsGreedily(getOperation(), std::move(patterns))))
    signalPassFailure();
}
