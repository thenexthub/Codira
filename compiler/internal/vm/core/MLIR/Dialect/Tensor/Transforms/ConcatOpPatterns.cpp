//===- ConcatOpPatterns.cpp - Patterns related to tensor.concat lowering --===//
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

#include "mlir/Dialect/Tensor/IR/Tensor.h"
#include "mlir/Dialect/Tensor/Transforms/Transforms.h"
#include "mlir/IR/PatternMatch.h"

using namespace mlir;
using namespace mlir::tensor;

namespace {

/// Decompose `tensor.concat` into `tensor.empty` and a chain of slice inserts.
///
/// %concat = tensor.concat dim(1) %0, %1 :
///         (tensor<2x3xf32>, tensor<2x4xf32>) -> tensor<2x7xf32>
///
/// Becomes
///
/// %empty = tensor.empty() : tensor<2x7xf32>
/// %insert0 = tensor.insert_slice %0 into %empty[0, 0][2, 3][1, 1]
/// %concat = tensor.insert_slice %1 into %insert0[0, 3][2, 4][1, 1]
struct DecomposeTensorConcatOp : public OpRewritePattern<ConcatOp> {
  using OpRewritePattern<ConcatOp>::OpRewritePattern;

  LogicalResult matchAndRewrite(ConcatOp concatOp,
                                PatternRewriter &rewriter) const override {
    FailureOr<SmallVector<Value>> decomposed =
        concatOp.decomposeOperation(rewriter);
    if (failed(decomposed)) {
      return rewriter.notifyMatchFailure(
          concatOp, "failed to get the decomposed insert slices");
    }
    rewriter.replaceOp(concatOp, decomposed.value()[0]);
    return success();
  }
};

} // namespace

void mlir::tensor::populateDecomposeTensorConcatPatterns(
    RewritePatternSet &patterns) {
  patterns.add<DecomposeTensorConcatOp>(patterns.getContext());
}
