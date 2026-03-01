//===- Split.cpp - Structured op splitting --------------------------------===//
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

#include "mlir/Dialect/Affine/IR/AffineOps.h"
#include "mlir/Dialect/Linalg/Transforms/Transforms.h"
#include "mlir/Dialect/Utils/StaticValueUtils.h"
#include "mlir/IR/AffineExpr.h"
#include "mlir/IR/Attributes.h"
#include "mlir/IR/BuiltinAttributes.h"
#include "mlir/IR/OpDefinition.h"
#include "mlir/Interfaces/TilingInterface.h"

#include "vm/core/ADT/STLExtras.h"
#include "vm/core/ADT/SmallVector.h"

using namespace mlir;
using namespace mlir::linalg;

/// Creates a part of the given `op` split along the iteration space `dimension`
/// with the given `size` and an optional `offset` (default 0). Makes slices
/// of operands, using the input operands of the original op and the output
/// operands provided as `resultOperands`. Expects `offsets` and `sizes` to
/// define the shape of the iteration space of the original op. Returns the
/// split-out op as well as the output operand values updated with the partial
/// results produced by this op through `results`.
static TilingInterface
createSplitPart(RewriterBase &b, Location loc, TilingInterface op,
                ArrayRef<OpFoldResult> offsets, ArrayRef<OpFoldResult> sizes,
                ValueRange resultOperands, unsigned dimension,
                OpFoldResult size, OpFoldResult offset,
                SmallVectorImpl<Value> &results) {
  // Iteration space of the current part.
  SmallVector<OpFoldResult> sizesCopy = toolchain::to_vector(sizes);
  SmallVector<OpFoldResult> offsetsCopy = toolchain::to_vector(offsets);
  sizesCopy[dimension] = size;
  offsetsCopy[dimension] = offset;

  // Create the part as if it were a single tile.
  FailureOr<TilingResult> tilingResult =
      op.getTiledImplementation(b, offsetsCopy, sizesCopy);

  // Insert the results back and populate the `results` list.
  for (auto [index, result] : toolchain::enumerate(tilingResult->tiledValues)) {
    SmallVector<OpFoldResult> resultOffsets, resultSizes;
    if (failed(op.getResultTilePosition(b, index, offsetsCopy, sizesCopy,
                                        resultOffsets, resultSizes)))
      return nullptr;
    SmallVector<OpFoldResult> resultStrides(resultOffsets.size(),
                                            b.getIndexAttr(1));
    Value inserted = tensor::InsertSliceOp::create(
        b, loc, result, resultOperands[index], resultOffsets, resultSizes,
        resultStrides);
    results.push_back(inserted);
  }
  // TODO: this part can be generalized maybe to not expect a single op.
  assert(tilingResult->tiledOps.size() == 1 &&
         "expected split part to return a single tiled operation");
  return cast<TilingInterface>(tilingResult->tiledOps[0]);
}

std::pair<TilingInterface, TilingInterface>
linalg::splitOp(RewriterBase &rewriter, TilingInterface op, unsigned dimension,
                OpFoldResult splitPoint) {
  // Compute the iteration space.
  SmallVector<Range> iterationSpace = op.getIterationDomain(rewriter);

  // Bail out on dimension overflow.
  if (dimension >= iterationSpace.size())
    return std::make_pair(op, TilingInterface());

  SmallVector<OpFoldResult> offsets = toolchain::to_vector(toolchain::map_range(
      iterationSpace, [](const Range &range) { return range.offset; }));
  SmallVector<OpFoldResult> sizes = toolchain::to_vector(toolchain::map_range(
      iterationSpace, [](const Range &range) { return range.size; }));

  // Adjust the split point so that it doesn't overflow the size.
  AffineExpr d0, d1, d2;
  bindDims(rewriter.getContext(), d0, d1, d2);
  OpFoldResult minSplitPoint = affine::makeComposedFoldedAffineMin(
      rewriter, op.getLoc(),
      AffineMap::inferFromExprList(ArrayRef<AffineExpr>{d0, d1 + d2},
                                   rewriter.getContext())
          .front(),
      {splitPoint, offsets[dimension], sizes[dimension]});

  // Compute the size of the second part. Return early if the second part would
  // have an empty iteration space.
  OpFoldResult remainingSize = affine::makeComposedFoldedAffineApply(
      rewriter, op.getLoc(), d0 + d1 - d2,
      {iterationSpace[dimension].offset, iterationSpace[dimension].size,
       minSplitPoint});
  if (auto attr = toolchain::dyn_cast_if_present<Attribute>(remainingSize)) {
    if (cast<IntegerAttr>(attr).getValue().isZero())
      return {op, TilingInterface()};
  }

  // Compute destination tensors.
  SmallVector<Value> destinationTensors;
  LogicalResult destStatus = tensor::getOrCreateDestinations(
      rewriter, op.getLoc(), op, destinationTensors);
  (void)destStatus;
  assert(succeeded(destStatus) && "failed to get destination tensors");

  // Create the first part.
  SmallVector<Value> firstResults;
  TilingInterface firstPart = createSplitPart(
      rewriter, op.getLoc(), op, offsets, sizes, destinationTensors, dimension,
      minSplitPoint, iterationSpace[dimension].offset, firstResults);

  // Need to pretend that the original op now takes as operands firstResults,
  // otherwise tiling interface implementation will take the wrong value to
  // produce data tiles.
  rewriter.modifyOpInPlace(op, [&]() {
    unsigned numTotalOperands = op->getNumOperands();
    unsigned numOutputOperands = firstResults.size();
    op->setOperands(numTotalOperands - numOutputOperands, numOutputOperands,
                    firstResults);
  });

  // Create the second part.
  OpFoldResult totalOffset = affine::makeComposedFoldedAffineApply(
      rewriter, op.getLoc(), d0 + d1, {offsets[dimension], minSplitPoint});
  SmallVector<Value> secondResults;
  TilingInterface secondPart =
      createSplitPart(rewriter, op.getLoc(), op, offsets, sizes, firstResults,
                      dimension, remainingSize, totalOffset, secondResults);

  // Propagate any errors in part creation.
  if (!firstPart || !secondPart)
    return {TilingInterface(), TilingInterface()};

  // Replace the original op with the results of the two newly created ops.
  rewriter.replaceOp(op, secondResults);
  return {firstPart, secondPart};
}
