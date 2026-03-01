//===- ShuffleRewriter.cpp - Implementation of shuffle rewriting  ---------===//
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
// This file implements in-dialect rewriting of the shuffle op for types i64 and
// f64, rewriting 64bit shuffles into two 32bit shuffles. This particular
// implementation using shifts and truncations can be obtained using clang: by
// emitting IR for shuffle operations with `-O3`.
//
//===----------------------------------------------------------------------===//

#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/GPU/IR/GPUDialect.h"
#include "mlir/Dialect/GPU/Transforms/Passes.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/PatternMatch.h"

using namespace mlir;

namespace {
struct GpuShuffleRewriter : public OpRewritePattern<gpu::ShuffleOp> {
  using OpRewritePattern<gpu::ShuffleOp>::OpRewritePattern;

  void initialize() {
    // Required as the pattern will replace the Op with 2 additional ShuffleOps.
    setHasBoundedRewriteRecursion();
  }
  LogicalResult matchAndRewrite(gpu::ShuffleOp op,
                                PatternRewriter &rewriter) const override {
    auto loc = op.getLoc();
    auto value = op.getValue();
    auto valueType = value.getType();
    auto valueLoc = value.getLoc();
    auto i32 = rewriter.getI32Type();
    auto i64 = rewriter.getI64Type();

    // If the type of the value is either i32 or f32, the op is already valid.
    if (!valueType.isIntOrFloat() || valueType.getIntOrFloatBitWidth() != 64)
      return rewriter.notifyMatchFailure(
          op, "only 64-bit int/float types are supported");

    Value lo, hi;

    // Float types must be converted to i64 to extract the bits.
    if (isa<FloatType>(valueType))
      value = arith::BitcastOp::create(rewriter, valueLoc, i64, value);

    // Get the low bits by trunc(value).
    lo = arith::TruncIOp::create(rewriter, valueLoc, i32, value);

    // Get the high bits by trunc(value >> 32).
    auto c32 = arith::ConstantOp::create(rewriter, valueLoc,
                                         rewriter.getIntegerAttr(i64, 32));
    hi = arith::ShRUIOp::create(rewriter, valueLoc, value, c32);
    hi = arith::TruncIOp::create(rewriter, valueLoc, i32, hi);

    // Shuffle the values.
    ValueRange loRes =
        gpu::ShuffleOp::create(rewriter, op.getLoc(), lo, op.getOffset(),
                               op.getWidth(), op.getMode())
            .getResults();
    ValueRange hiRes =
        gpu::ShuffleOp::create(rewriter, op.getLoc(), hi, op.getOffset(),
                               op.getWidth(), op.getMode())
            .getResults();

    // Convert lo back to i64.
    lo = arith::ExtUIOp::create(rewriter, valueLoc, i64, loRes[0]);

    // Convert hi back to i64.
    hi = arith::ExtUIOp::create(rewriter, valueLoc, i64, hiRes[0]);
    hi = arith::ShLIOp::create(rewriter, valueLoc, hi, c32);

    // Obtain the shuffled bits hi | lo.
    value = arith::OrIOp::create(rewriter, loc, hi, lo);

    // Convert the value back to float.
    if (isa<FloatType>(valueType))
      value = arith::BitcastOp::create(rewriter, valueLoc, valueType, value);

    // Obtain the shuffle validity by combining both validities.
    auto validity = arith::AndIOp::create(rewriter, loc, loRes[1], hiRes[1]);

    // Replace the op.
    rewriter.replaceOp(op, {value, validity});
    return success();
  }
};
} // namespace

void mlir::populateGpuShufflePatterns(RewritePatternSet &patterns) {
  patterns.add<GpuShuffleRewriter>(patterns.getContext());
}
