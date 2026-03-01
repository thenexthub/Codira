//===- PromoteShuffleToAMDGPU.cpp - Promote shuffle to AMDGPU -------------===//
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
// This file contains patterns to try to promote `gpu.shuffle`s to specialized
// AMDGPU intrinsics.
//
//===----------------------------------------------------------------------===//

#include "mlir/Dialect/AMDGPU/Utils/Chipset.h"
#include "mlir/Dialect/GPU/Transforms/Passes.h"

#include "mlir/Dialect/AMDGPU/IR/AMDGPUDialect.h"
#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/GPU/IR/GPUDialect.h"
#include "mlir/IR/PatternMatch.h"
#include <optional>

using namespace mlir;

namespace {

constexpr amdgpu::Chipset kGfx950 = amdgpu::Chipset(9, 5, 0);

/// Try to promote `gpu.shuffle` to `amdgpu.swizzle_bitmode`, width must be 64
/// and offset must be a constant integer in the range [0, 31].
struct PromoteShuffleToSwizzlePattern
    : public OpRewritePattern<gpu::ShuffleOp> {
  using OpRewritePattern::OpRewritePattern;

  LogicalResult matchAndRewrite(gpu::ShuffleOp op,
                                PatternRewriter &rewriter) const override {
    if (op.getMode() != gpu::ShuffleMode::XOR)
      return rewriter.notifyMatchFailure(op,
                                         "only xor shuffle mode is supported");

    if (!isConstantIntValue(op.getWidth(), 64))
      return rewriter.notifyMatchFailure(op,
                                         "only 64 width shuffle is supported");

    std::optional<int64_t> offset = getConstantIntValue(op.getOffset());
    if (!offset)
      return rewriter.notifyMatchFailure(op,
                                         "offset must be a constant integer");

    int64_t offsetValue = *offset;
    if (offsetValue < 0 || offsetValue >= 32)
      return rewriter.notifyMatchFailure(op,
                                         "offset must be in the range [0, 31]");

    Location loc = op.getLoc();
    Value res = amdgpu::SwizzleBitModeOp::create(
        rewriter, loc, op.getResult(0).getType(), op.getValue(), /*andMask=*/31,
        /*orMask=*/0, /*xorMask=*/offsetValue);
    Value valid = arith::ConstantIntOp::create(rewriter, loc, 1, /*width*/ 1);
    rewriter.replaceOp(op, {res, valid});
    return success();
  }
};

/// Try to promote `gpu.shuffle` to `amdgpu.permlane_swap`, width must be 64
/// and offset must be a constant integer in the set {16, 32}.
struct PromoteShuffleToPermlanePattern
    : public OpRewritePattern<gpu::ShuffleOp> {
  using OpRewritePattern::OpRewritePattern;

  LogicalResult matchAndRewrite(gpu::ShuffleOp op,
                                PatternRewriter &rewriter) const override {
    if (op.getMode() != gpu::ShuffleMode::XOR)
      return rewriter.notifyMatchFailure(op,
                                         "only xor shuffle mode is supported");

    if (!isConstantIntValue(op.getWidth(), 64))
      return rewriter.notifyMatchFailure(op,
                                         "only 64 width shuffle is supported");

    std::optional<int64_t> offset = getConstantIntValue(op.getOffset());
    if (!offset)
      return rewriter.notifyMatchFailure(op,
                                         "offset must be a constant integer");

    int64_t offsetValue = *offset;
    if (offsetValue != 16 && offsetValue != 32)
      return rewriter.notifyMatchFailure(op, "offset must be either 15 or 31");

    Location loc = op.getLoc();
    Value res = amdgpu::PermlaneSwapOp::create(
        rewriter, loc, op.getResult(0).getType(), op.getValue(), offsetValue);
    Value valid = arith::ConstantIntOp::create(rewriter, loc, 1, /*width*/ 1);
    rewriter.replaceOp(op, {res, valid});
    return success();
  }
};

} // namespace

void mlir::populateGpuPromoteShuffleToAMDGPUPatterns(
    RewritePatternSet &patterns, std::optional<amdgpu::Chipset> maybeChipset) {
  patterns.add<PromoteShuffleToSwizzlePattern>(patterns.getContext(),
                                               /*benefit*/ 1);
  if (maybeChipset && *maybeChipset >= kGfx950)
    patterns.add<PromoteShuffleToPermlanePattern>(patterns.getContext(),
                                                  /*benefit*/ 2);
}
