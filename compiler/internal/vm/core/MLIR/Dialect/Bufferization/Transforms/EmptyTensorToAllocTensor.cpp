//===- InitTensorToAllocTensor.cpp - Lower tensor.empty to alloc_tensor ---===//
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

#include "mlir/Dialect/Bufferization/Transforms/Passes.h"

#include "mlir/Dialect/Bufferization/IR/Bufferization.h"
#include "mlir/Dialect/Bufferization/Transforms/Transforms.h"
#include "mlir/Dialect/Tensor/IR/Tensor.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"

namespace mlir {
namespace bufferization {
#define GEN_PASS_DEF_EMPTYTENSORTOALLOCTENSORPASS
#include "mlir/Dialect/Bufferization/Transforms/Passes.h.inc"
} // namespace bufferization
} // namespace mlir

using namespace mlir;
using namespace mlir::bufferization;
using namespace mlir::tensor;

namespace {
struct EmptyTensorLoweringPattern : public OpRewritePattern<tensor::EmptyOp> {
  using OpRewritePattern<tensor::EmptyOp>::OpRewritePattern;

  LogicalResult matchAndRewrite(tensor::EmptyOp op,
                                PatternRewriter &rewriter) const override {
    rewriter.replaceOpWithNewOp<bufferization::AllocTensorOp>(
        op, op.getType(), op.getDynamicSizes());
    return success();
  }
};

struct EmptyTensorToAllocTensor
    : public bufferization::impl::EmptyTensorToAllocTensorPassBase<
          EmptyTensorToAllocTensor> {
  void runOnOperation() override;

  void getDependentDialects(DialectRegistry &registry) const override {
    registry
        .insert<tensor::TensorDialect, bufferization::BufferizationDialect>();
  }
};
} // namespace

void bufferization::populateEmptyTensorToAllocTensorPattern(
    RewritePatternSet &patterns) {
  patterns.insert<EmptyTensorLoweringPattern>(patterns.getContext());
}

void EmptyTensorToAllocTensor::runOnOperation() {
  Operation *op = getOperation();
  RewritePatternSet patterns(op->getContext());
  populateEmptyTensorToAllocTensorPattern(patterns);
  if (failed(applyPatternsGreedily(op, std::move(patterns))))
    signalPassFailure();
}
