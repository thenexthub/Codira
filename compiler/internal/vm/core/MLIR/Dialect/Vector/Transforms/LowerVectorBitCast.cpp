//===- LowerVectorBitCast.cpp - Lower 'vector.bitcast' operation ----------===//
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
// This file implements target-independent rewrites and utilities to lower the
// 'vector.bitcast' operation.
//
//===----------------------------------------------------------------------===//

#include "mlir/Dialect/UB/IR/UBOps.h"
#include "mlir/Dialect/Vector/IR/VectorOps.h"
#include "mlir/Dialect/Vector/Transforms/LoweringPatterns.h"
#include "mlir/Dialect/Vector/Utils/VectorUtils.h"
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/PatternMatch.h"

#define DEBUG_TYPE "vector-bitcast-lowering"

using namespace mlir;
using namespace mlir::vector;

namespace {

/// A one-shot unrolling of vector.bitcast to the `targetRank`.
///
/// Example:
///
///   vector.bitcast %a, %b : vector<1x2x3x4xi64> to vector<1x2x3x8xi32>
///
/// Would be unrolled to:
///
/// %result = ub.poison : vector<1x2x3x8xi32>
/// %0 = vector.extract %a[0, 0, 0]                 ─┐
///        : vector<4xi64> from vector<1x2x3x4xi64>  |
/// %1 = vector.bitcast %0                           | - Repeated 6x for
///        : vector<4xi64> to vector<8xi32>          |   all leading positions
/// %2 = vector.insert %1, %result [0, 0, 0]         |
///        : vector<8xi64> into vector<1x2x3x8xi32> ─┘
///
/// Note: If any leading dimension before the `targetRank` is scalable the
/// unrolling will stop before the scalable dimension.
class UnrollBitCastOp final : public OpRewritePattern<vector::BitCastOp> {
public:
  UnrollBitCastOp(int64_t targetRank, MLIRContext *context,
                  PatternBenefit benefit = 1)
      : OpRewritePattern(context, benefit), targetRank(targetRank) {};

  LogicalResult matchAndRewrite(vector::BitCastOp op,
                                PatternRewriter &rewriter) const override {
    VectorType resultType = op.getResultVectorType();
    auto unrollIterator = vector::createUnrollIterator(resultType, targetRank);
    if (!unrollIterator)
      return failure();

    auto unrollRank = unrollIterator->getRank();
    ArrayRef<int64_t> shape = resultType.getShape().drop_front(unrollRank);
    ArrayRef<bool> scalableDims =
        resultType.getScalableDims().drop_front(unrollRank);
    auto bitcastResType =
        VectorType::get(shape, resultType.getElementType(), scalableDims);

    Location loc = op.getLoc();
    Value result = ub::PoisonOp::create(rewriter, loc, resultType);
    for (auto position : *unrollIterator) {
      Value extract =
          vector::ExtractOp::create(rewriter, loc, op.getSource(), position);
      Value bitcast =
          vector::BitCastOp::create(rewriter, loc, bitcastResType, extract);
      result =
          vector::InsertOp::create(rewriter, loc, bitcast, result, position);
    }

    rewriter.replaceOp(op, result);
    return success();
  }

private:
  int64_t targetRank = 1;
};

} // namespace

void mlir::vector::populateVectorBitCastLoweringPatterns(
    RewritePatternSet &patterns, int64_t targetRank, PatternBenefit benefit) {
  patterns.add<UnrollBitCastOp>(targetRank, patterns.getContext(), benefit);
}
