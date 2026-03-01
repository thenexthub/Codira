//===- StridedMetadataRangeAnalysis.cpp - Integer range analysis --------*- C++
//-*-===//
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
// This file defines the dataflow analysis class for integer range inference
// which is used in transformations over the `arith` dialect such as
// branch elimination or signed->unsigned rewriting
//
//===----------------------------------------------------------------------===//

#include "mlir/Analysis/DataFlow/StridedMetadataRangeAnalysis.h"
#include "mlir/Analysis/DataFlow/IntegerRangeAnalysis.h"
#include "mlir/Dialect/Utils/IndexingUtils.h"
#include "mlir/IR/Operation.h"
#include "mlir/IR/Value.h"
#include "mlir/Support/DebugStringHelper.h"
#include "vm/core/Support/Debug.h"
#include "vm/core/Support/DebugLog.h"

#define DEBUG_TYPE "strided-metadata-range-analysis"

using namespace mlir;
using namespace mlir::dataflow;

/// Get the entry state for a value. For any value that is not a ranked memref,
/// this function sets the metadata to a top state with no offsets, sizes, or
/// strides. For `memref` types, this function will use the metadata in the type
/// to try to deduce as much informaiton as possible.
static StridedMetadataRange getEntryStateImpl(Value v, int32_t indexBitwidth) {
  // TODO: generalize this method with a type interface.
  auto mTy = dyn_cast<BaseMemRefType>(v.getType());

  // If not a memref or it's un-ranked, don't infer any metadata.
  if (!mTy || !mTy.hasRank())
    return StridedMetadataRange::getMaxRanges(indexBitwidth, 0, 0, 0);

  // Get the top state.
  auto metadata =
      StridedMetadataRange::getMaxRanges(indexBitwidth, mTy.getRank());

  // Compute the offset and strides.
  int64_t offset;
  SmallVector<int64_t> strides;
  if (failed(cast<MemRefType>(mTy).getStridesAndOffset(strides, offset)))
    return metadata;

  // Refine the metadata if we know it from the type.
  if (!ShapedType::isDynamic(offset)) {
    metadata.getOffsets()[0] =
        ConstantIntRanges::constant(APInt(indexBitwidth, offset));
  }
  for (auto &&[size, range] :
       toolchain::zip_equal(mTy.getShape(), metadata.getSizes())) {
    if (ShapedType::isDynamic(size))
      continue;
    range = ConstantIntRanges::constant(APInt(indexBitwidth, size));
  }
  for (auto &&[stride, range] :
       toolchain::zip_equal(strides, metadata.getStrides())) {
    if (ShapedType::isDynamic(stride))
      continue;
    range = ConstantIntRanges::constant(APInt(indexBitwidth, stride));
  }

  return metadata;
}

StridedMetadataRangeAnalysis::StridedMetadataRangeAnalysis(
    DataFlowSolver &solver, int32_t indexBitwidth)
    : SparseForwardDataFlowAnalysis(solver), indexBitwidth(indexBitwidth) {
  assert(indexBitwidth > 0 && "invalid bitwidth");
}

void StridedMetadataRangeAnalysis::setToEntryState(
    StridedMetadataRangeLattice *lattice) {
  propagateIfChanged(lattice, lattice->join(getEntryStateImpl(
                                  lattice->getAnchor(), indexBitwidth)));
}

LogicalResult StridedMetadataRangeAnalysis::visitOperation(
    Operation *op, ArrayRef<const StridedMetadataRangeLattice *> operands,
    ArrayRef<StridedMetadataRangeLattice *> results) {
  auto inferrable = dyn_cast<InferStridedMetadataOpInterface>(op);

  // Bail if we cannot reason about the op.
  if (!inferrable) {
    setAllToEntryStates(results);
    return success();
  }

  LDBG() << "Inferring metadata for: "
         << OpWithFlags(op, OpPrintingFlags().skipRegions());

  // Helper function to retrieve int range values.
  auto getIntRange = [&](Value value) -> IntegerValueRange {
    auto lattice = getOrCreateFor<IntegerValueRangeLattice>(
        getProgramPointAfter(op), value);
    return lattice ? lattice->getValue() : IntegerValueRange();
  };

  // Convert the arguments lattices to a vector.
  SmallVector<StridedMetadataRange> argRanges = toolchain::map_to_vector(
      operands, [](const StridedMetadataRangeLattice *lattice) {
        return lattice->getValue();
      });

  // Callback to set metadata on a result.
  auto joinCallback = [&](Value v, const StridedMetadataRange &md) {
    auto result = cast<OpResult>(v);
    assert(toolchain::is_contained(op->getResults(), result));
    LDBG() << "- Inferred metadata: " << md;
    StridedMetadataRangeLattice *lattice = results[result.getResultNumber()];
    ChangeResult changed = lattice->join(md);
    LDBG() << "- Joined metadata: " << lattice->getValue();
    propagateIfChanged(lattice, changed);
  };

  // Infer the metadata.
  inferrable.inferStridedMetadataRanges(argRanges, getIntRange, joinCallback,
                                        indexBitwidth);
  return success();
}
