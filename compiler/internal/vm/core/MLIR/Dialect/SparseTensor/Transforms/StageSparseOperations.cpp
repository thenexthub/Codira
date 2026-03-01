//===- StageSparseOperations.cpp - stage sparse ops rewriting rules -------===//
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

#include "mlir/Dialect/Bufferization/IR/Bufferization.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/SparseTensor/IR/SparseTensor.h"
#include "mlir/Dialect/SparseTensor/Transforms/Passes.h"

using namespace mlir;
using namespace mlir::sparse_tensor;

namespace {

struct GuardSparseAlloc
    : public OpRewritePattern<bufferization::AllocTensorOp> {
  using OpRewritePattern<bufferization::AllocTensorOp>::OpRewritePattern;

  LogicalResult matchAndRewrite(bufferization::AllocTensorOp op,
                                PatternRewriter &rewriter) const override {
    // Only rewrite sparse allocations.
    if (!getSparseTensorEncoding(op.getResult().getType()))
      return failure();

    // Only rewrite sparse allocations that escape the method
    // without any chance of a finalizing operation in between.
    // Here we assume that sparse tensor setup never crosses
    // method boundaries. The current rewriting only repairs
    // the most obvious allocate-call/return cases.
    if (!toolchain::all_of(op->getUses(), [](OpOperand &use) {
          return isa<func::ReturnOp, func::CallOp, func::CallIndirectOp>(
              use.getOwner());
        }))
      return failure();

    // Guard escaping empty sparse tensor allocations with a finalizing
    // operation that leaves the underlying storage in a proper state
    // before the tensor escapes across the method boundary.
    rewriter.setInsertionPointAfter(op);
    auto load = LoadOp::create(rewriter, op.getLoc(), op.getResult(), true);
    rewriter.replaceAllUsesExcept(op, load, load);
    return success();
  }
};

template <typename StageWithSortOp>
struct StageUnorderedSparseOps : public OpRewritePattern<StageWithSortOp> {
  using OpRewritePattern<StageWithSortOp>::OpRewritePattern;

  LogicalResult matchAndRewrite(StageWithSortOp op,
                                PatternRewriter &rewriter) const override {
    Location loc = op.getLoc();
    Value tmpBuf = nullptr;
    auto itOp = toolchain::cast<StageWithSortSparseOp>(op.getOperation());
    LogicalResult stageResult = itOp.stageWithSort(rewriter, tmpBuf);
    // Deallocate tmpBuf.
    // TODO: Delegate to buffer deallocation pass in the future.
    if (succeeded(stageResult) && tmpBuf)
      bufferization::DeallocTensorOp::create(rewriter, loc, tmpBuf);

    return stageResult;
  }
};
} // namespace

void mlir::populateStageSparseOperationsPatterns(RewritePatternSet &patterns) {
  patterns.add<GuardSparseAlloc, StageUnorderedSparseOps<ConvertOp>,
               StageUnorderedSparseOps<ConcatenateOp>>(patterns.getContext());
}
