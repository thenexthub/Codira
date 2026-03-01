//===- VectorMaskElimination.cpp - Eliminate Vector Masks -----------------===//
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

#include "mlir/Dialect/Utils/StaticValueUtils.h"
#include "mlir/Dialect/Vector/IR/ScalableValueBoundsConstraintSet.h"
#include "mlir/Dialect/Vector/Transforms/VectorTransforms.h"
#include "mlir/Interfaces/FunctionInterfaces.h"

using namespace mlir;
using namespace mlir::vector;
namespace {

/// Attempts to resolve a (scalable) CreateMaskOp to an all-true constant mask.
/// All-true masks can then be eliminated by simple folds.
LogicalResult resolveAllTrueCreateMaskOp(IRRewriter &rewriter,
                                         vector::CreateMaskOp createMaskOp,
                                         VscaleRange vscaleRange) {
  auto maskType = createMaskOp.getVectorType();
  auto maskTypeDimScalableFlags = maskType.getScalableDims();
  auto maskTypeDimSizes = maskType.getShape();

  struct UnknownMaskDim {
    size_t position;
    Value dimSize;
  };

  // Loop over the CreateMaskOp operands and collect unknown dims (i.e. dims
  // that are not obviously constant). If any constant dimension is not all-true
  // bail out early (as this transform only trying to resolve all-true masks).
  // This avoids doing value-bounds anaylis in cases like:
  // `%mask = vector.create_mask %dynamicValue, %c2 : vector<8x4xi1>`
  // ...where it is known the mask is not all-true by looking at `%c2`.
  SmallVector<UnknownMaskDim> unknownDims;
  for (auto [i, dimSize] : toolchain::enumerate(createMaskOp.getOperands())) {
    if (auto intSize = getConstantIntValue(dimSize)) {
      // Mask not all-true for this dim.
      if (maskTypeDimScalableFlags[i] || intSize < maskTypeDimSizes[i])
        return failure();
    } else if (auto vscaleMultiplier = getConstantVscaleMultiplier(dimSize)) {
      // Mask not all-true for this dim.
      if (vscaleMultiplier < maskTypeDimSizes[i])
        return failure();
    } else {
      // Unknown (without further analysis).
      unknownDims.push_back(UnknownMaskDim{i, dimSize});
    }
  }

  for (auto [i, dimSize] : unknownDims) {
    // Compute the lower bound for the unknown dimension (i.e. the smallest
    // value it could be).
    FailureOr<ConstantOrScalableBound> dimLowerBound =
        vector::ScalableValueBoundsConstraintSet::computeScalableBound(
            dimSize, {}, vscaleRange.vscaleMin, vscaleRange.vscaleMax,
            presburger::BoundType::LB);
    if (failed(dimLowerBound))
      return failure();
    auto dimLowerBoundSize = dimLowerBound->getSize();
    if (failed(dimLowerBoundSize))
      return failure();
    if (dimLowerBoundSize->scalable) {
      // 1. The lower bound, LB, is scalable. If LB is < the mask dim size then
      // this dim is not all-true.
      if (dimLowerBoundSize->baseSize < maskTypeDimSizes[i])
        return failure();
    } else {
      // 2. The lower bound, LB, is a constant.
      // - If the mask dim size is scalable then this dim is not all-true.
      if (maskTypeDimScalableFlags[i])
        return failure();
      // - If LB < the _fixed-size_ mask dim size then this dim is not all-true.
      if (dimLowerBoundSize->baseSize < maskTypeDimSizes[i])
        return failure();
    }
  }

  // Replace createMaskOp with an all-true constant. This should result in the
  // mask being removed in most cases (as xfer ops + vector.mask have folds to
  // remove all-true masks).
  auto allTrue = vector::ConstantMaskOp::create(
      rewriter, createMaskOp.getLoc(), maskType, ConstantMaskKind::AllTrue);
  rewriter.replaceAllUsesWith(createMaskOp, allTrue);
  return success();
}

} // namespace

namespace mlir::vector {

void eliminateVectorMasks(IRRewriter &rewriter, FunctionOpInterface function,
                          std::optional<VscaleRange> vscaleRange) {
  // TODO: Support fixed-size case. This is less likely to be useful as for
  // fixed-size code dimensions are all static so masks tend to fold away.
  if (!vscaleRange)
    return;

  // Early exit for functions without a body.
  if (function.isExternal())
    return;

  OpBuilder::InsertionGuard g(rewriter);

  // Build worklist so we can safely insert new ops in
  // `resolveAllTrueCreateMaskOp()`.
  SmallVector<vector::CreateMaskOp> worklist;
  function.walk([&](vector::CreateMaskOp createMaskOp) {
    worklist.push_back(createMaskOp);
  });

  rewriter.setInsertionPointToStart(&function.front());
  for (auto mask : worklist)
    (void)resolveAllTrueCreateMaskOp(rewriter, mask, *vscaleRange);
}

} // namespace mlir::vector
