//===- BubbleDownMemorySpaceCasts.cpp - Bubble down casts transform -------===//
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

#include "mlir/Transforms/BubbleDownMemorySpaceCasts.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Interfaces/MemOpInterfaces.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"
#include "mlir/Transforms/Passes.h"
#include "vm/core/Support/Debug.h"

using namespace mlir;

namespace mlir {
#define GEN_PASS_DEF_BUBBLEDOWNMEMORYSPACECASTS
#include "mlir/Transforms/Passes.h.inc"
} // namespace mlir

namespace {
//===----------------------------------------------------------------------===//
// BubbleDownCastsPattern pattern
//===----------------------------------------------------------------------===//
/// Pattern to bubble down casts into consumer operations.
struct BubbleDownCastsPattern
    : public OpInterfaceRewritePattern<MemorySpaceCastConsumerOpInterface> {
  using OpInterfaceRewritePattern::OpInterfaceRewritePattern;

  LogicalResult matchAndRewrite(MemorySpaceCastConsumerOpInterface op,
                                PatternRewriter &rewriter) const override {
    FailureOr<std::optional<SmallVector<Value>>> results =
        op.bubbleDownCasts(rewriter);
    if (failed(results))
      return failure();
    if (!results->has_value()) {
      rewriter.modifyOpInPlace(op, []() {});
      return success();
    }
    rewriter.replaceOp(op, **results);
    return success();
  }
};

//===----------------------------------------------------------------------===//
// BubbleDownMemorySpaceCasts pass
//===----------------------------------------------------------------------===//

struct BubbleDownMemorySpaceCasts
    : public impl::BubbleDownMemorySpaceCastsBase<BubbleDownMemorySpaceCasts> {
  using impl::BubbleDownMemorySpaceCastsBase<
      BubbleDownMemorySpaceCasts>::BubbleDownMemorySpaceCastsBase;

  void runOnOperation() override {
    RewritePatternSet patterns(&getContext());
    populateBubbleDownMemorySpaceCastPatterns(patterns, PatternBenefit(1));
    if (failed(applyPatternsGreedily(getOperation(), std::move(patterns))))
      signalPassFailure();
  }
};
} // namespace

void mlir::populateBubbleDownMemorySpaceCastPatterns(
    RewritePatternSet &patterns, PatternBenefit benefit) {
  patterns.add<BubbleDownCastsPattern>(patterns.getContext(), benefit);
}
