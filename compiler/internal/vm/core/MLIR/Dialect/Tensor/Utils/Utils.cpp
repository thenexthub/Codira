//===- Utils.cpp - Utilities to support the Tensor dialect ----------------===//
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
// This file implements utilities for the Tensor dialect.
//
//===----------------------------------------------------------------------===//

#include "mlir/Dialect/Tensor/Utils/Utils.h"

#include "mlir/Dialect/Affine/IR/AffineOps.h"
#include "mlir/Dialect/Arith/Utils/Utils.h"
#include "mlir/Dialect/Utils/IndexingUtils.h"
#include "mlir/Interfaces/ValueBoundsOpInterface.h"

using namespace mlir;
using namespace mlir::tensor;

PadOp mlir::tensor::createPadHighOp(RankedTensorType resType, Value source,
                                    Value pad, bool nofold, Location loc,
                                    OpBuilder &b, ValueRange dynOutDims) {

  // This assumption simplifies the following logic without limiting what's
  // required _today_. If needed, we can relax it in the future.
  assert(((resType.getNumDynamicDims() == dynOutDims.size()) ||
          dynOutDims.empty()) &&
         "Either none or all output dynamic dims must be specified!");

  // Init "low" and "high" padding values ("low" is kept as is, "high" is
  // computed below).
  SmallVector<OpFoldResult> low(resType.getRank(), b.getIndexAttr(0));
  SmallVector<OpFoldResult> high(resType.getRank(), b.getIndexAttr(0));

  size_t outDimIdx = 0;

  for (const auto [idx, val] : enumerate(resType.getShape())) {
    bool isDimDynamic = ShapedType::isDynamic(val);
    bool updatePadHigh = !isDimDynamic || !dynOutDims.empty();

    // Keep the default padding width (i.e. "0") when the output dim is dynamic
    // and no actual output sizes have been provided.
    if (!updatePadHigh)
      continue;

    // Compute the padding width: resDim - sourceDim.
    AffineExpr d0, d1;
    bindDims(b.getContext(), d0, d1);
    OpFoldResult sourceDim = tensor::getMixedSize(b, loc, source, idx);
    OpFoldResult outDim = isDimDynamic ? OpFoldResult(dynOutDims[outDimIdx++])
                                       : OpFoldResult(b.getIndexAttr(val));

    high[idx] = affine::makeComposedFoldedAffineApply(b, loc, d0 - d1,
                                                      {outDim, sourceDim});
  }
  return PadOp::create(b, loc, resType, source, low, high, pad, nofold);
}

SmallVector<Value> mlir::tensor::createDynamicDimValues(OpBuilder &b,
                                                        Location loc,
                                                        Value rankedTensor) {
  auto tensorTy = cast<RankedTensorType>(rankedTensor.getType());
  SmallVector<Value> dynamicDims;
  for (const auto &en : toolchain::enumerate(tensorTy.getShape())) {
    if (en.value() == ShapedType::kDynamic)
      dynamicDims.push_back(
          tensor::DimOp::create(b, loc, rankedTensor, en.index()));
  }
  return dynamicDims;
}

FailureOr<RankedTensorType>
mlir::tensor::computeTransposedType(RankedTensorType rankedTensorType,
                                    ArrayRef<int64_t> transposeVector) {
  if (transposeVector.empty())
    return rankedTensorType;

  if (!isPermutationVector(transposeVector) ||
      transposeVector.size() != static_cast<size_t>(rankedTensorType.getRank()))
    return failure();

  SmallVector<int64_t> transposedShape(rankedTensorType.getShape());
  applyPermutationToVector(transposedShape, transposeVector);

  using RTTBuilder = RankedTensorType::Builder;
  RankedTensorType transposedTensorType =
      RTTBuilder(rankedTensorType).setShape(transposedShape);
  return transposedTensorType;
}

CollapseShapeOp
mlir::tensor::dropGivenUnitDims(OpBuilder &b, Location loc, Value src,
                                const toolchain::SmallBitVector &dropDims) {
  auto srcType = cast<ShapedType>(src.getType());
  int64_t rank = srcType.getRank();
  assert(rank == static_cast<int64_t>(dropDims.size()) &&
         "dropDims dimension does not match src tensor rank");
  assert(toolchain::all_of(
             dropDims.set_bits(),
             [&](unsigned dim) { return srcType.getShape()[dim] == 1; }) &&
         "Dropping non unit dimension");
  // Computed reassociation map for the corresponding tensor.collapse_shape.
  SmallVector<ReassociationIndices, 2> reassocMaps;
  // Current reassociation group to add dropped dimension to.

  int64_t nextDimToGroup = 0;
  toolchain::SmallBitVector keptDims(dropDims);
  keptDims.flip();
  int64_t lastSetBit = keptDims.find_last();
  for (int64_t setBit : keptDims.set_bits()) {
    // Group consecutive dropped dimension with the next non-dropped dimension.
    // If this is the last set dimension, also group all subsequent dropped
    // dimension, if any.
    int64_t upTo = setBit == lastSetBit ? rank - 1 : setBit;
    auto seq = toolchain::seq_inclusive(nextDimToGroup, upTo);
    reassocMaps.emplace_back(toolchain::make_range(seq.begin(), seq.end()));
    nextDimToGroup = setBit + 1;
  }
  return tensor::CollapseShapeOp::create(b, loc, src, reassocMaps);
}

bool mlir::tensor::isCastLikeInsertSliceOp(InsertSliceOp op) {
  toolchain::SmallBitVector droppedDims = op.getDroppedDims();
  int64_t srcDim = 0;
  RankedTensorType resultType = op.getDestType();
  // Source dims and destination dims (apart from dropped dims) must have the
  // same size.
  for (int64_t resultDim = 0; resultDim < resultType.getRank(); ++resultDim) {
    if (droppedDims.test(resultDim)) {
      // InsertSlice may expand unit dimensions that result from inserting a
      // size-1 slice into a non-size-1 result dimension.
      if (resultType.getDimSize(resultDim) != 1)
        return false;
      continue;
    }
    FailureOr<bool> equalDimSize = ValueBoundsConstraintSet::areEqual(
        {op.getSource(), srcDim}, {op.getResult(), resultDim});
    if (failed(equalDimSize) || !*equalDimSize)
      return false;
    ++srcDim;
  }

  return true;
}

bool mlir::tensor::isCastLikeExtractSliceOp(ExtractSliceOp op) {
  toolchain::SmallBitVector droppedDims = op.getDroppedDims();
  int64_t resultDim = 0;
  // Source dims and result dims (apart from dropped dims) must have the same
  // size.
  RankedTensorType sourceType = op.getSourceType();
  for (int64_t dim = 0, e = sourceType.getRank(); dim < e; ++dim) {
    if (droppedDims.test(dim)) {
      // ExtractSlice may drop unit dimensions that result from taking a size-1
      // slice from a non-size-1 source dimension.
      if (sourceType.getDimSize(dim) != 1)
        return false;
      continue;
    }
    FailureOr<bool> equalDimSize = ValueBoundsConstraintSet::areEqual(
        {op.getSource(), dim}, {op.getResult(), resultDim});
    if (failed(equalDimSize) || !*equalDimSize)
      return false;
    ++resultDim;
  }

  return true;
}
