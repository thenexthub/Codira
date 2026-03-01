//===- IndexingMapOpInterface.cpp -- IndexingMapOpInterface impl ----------===//
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

#include "mlir/Interfaces/IndexingMapOpInterface.h"

using namespace mlir;

namespace mlir {
#include "mlir/Interfaces/IndexingMapOpInterface.cpp.inc"
} // namespace mlir

LogicalResult mlir::IndexingMapOpInterface::verifyImpl() {
  // All input/output operands must be indexed.
  if (static_cast<int64_t>(getIndexingMapsArray().size()) !=
      getOperation()->getNumOperands())
    return this->emitOpError("expected the number of indexing_map (")
           << getIndexingMapsArray().size()
           << ") to be equal to the number of input/output operands ("
           << getOperation()->getNumOperands() << ")";

  SmallVector<int64_t> allShapesSizes;

  for (OpOperand &opOperand : getOperation()->getOpOperands()) {
    AffineMap indexingMap = getMatchingIndexingMap(&opOperand);
    SmallVector<int64_t> shape = getStaticOperandShape(&opOperand);
    int64_t rank = shape.size();

    // Symbols disallowed.
    if (indexingMap.getNumSymbols() != 0)
      return this->emitOpError("unexpected symbols in indexing_map #")
             << opOperand.getOperandNumber();

    // Result rank must match operand rank.
    if (indexingMap.getNumResults() != rank)
      return this->emitOpError("expected operand #")
             << opOperand.getOperandNumber() << " rank (" << rank
             << ") to match the result rank of indexing_map ("
             << indexingMap.getNumResults() << ")";

    toolchain::append_range(allShapesSizes, shape);
  }

  AffineMap invertedMap = getShapesToLoopsMap();
  if (!invertedMap) {
    std::string str;
    toolchain::raw_string_ostream os(str);
    getLoopsToShapesMap().print(os);
    return this->emitOpError("invalid indexing maps are non-invertible: ")
           << "(" << str << ")";
  }

  SmallVector<int64_t> endLoopRangeValues = invertedMap.compose(allShapesSizes);

  // Check if given shapes match to inferred shapes.
  SmallVector<int64_t> startLoopRangeValues(endLoopRangeValues.size(), 0);
  // Verify only static cases since we can't get exact dimension sizes and
  // loop ranges for dynamic cases in this stage.
  if (toolchain::none_of(endLoopRangeValues, ShapedType::isDynamic)) {
    // Exclusive end range.
    for (int64_t &range : endLoopRangeValues)
      range -= 1;
    for (OpOperand &opOperand : getOperation()->getOpOperands()) {
      AffineMap indexingMap = getMatchingIndexingMap(&opOperand);
      SmallVector<int64_t> startIndices =
          indexingMap.compose(startLoopRangeValues);
      SmallVector<int64_t> endIndices = indexingMap.compose(endLoopRangeValues);
      SmallVector<int64_t> shape = getStaticOperandShape(&opOperand);
      for (auto dim : toolchain::seq<int64_t>(0, shape.size())) {
        // Ignore dynamic dimension or the case that the dimension size is 0
        if (ShapedType::isDynamic(shape[dim]) || shape[dim] == 0)
          continue;

        // The first index or last index should be the maximum or the minimum in
        // the inferred index ranges since the range is increasing or
        // decreasing. The size of dimensions of input/output operands and the
        // maximum value + 1 in the inferred range should be the same. But, for
        // now we check if the inferred ranges are in boundary of input/output
        // operands' size or not in case that Affine Expressions are complicated
        // such as d0 * 3
        // + d1 since it is not easy to handle the issues.
        // Found the case that this solution can't check, for example, (d0, d1)
        // -> (d1 - d0)
        int64_t inferredDimSize =
            std::max(startIndices[dim], endIndices[dim]) + 1;
        if (std::min(startIndices[dim], endIndices[dim]) < 0) {
          std::string mapStr;
          {
            toolchain::raw_string_ostream os(mapStr);
            os << indexingMap;
          }
          return this->emitOpError(
                     "unexpected result less than 0 at expression #")
                 << dim << " in " << mapStr;
        }
        if (isa<AffineDimExpr>(indexingMap.getResult(dim))) {
          if (inferredDimSize != shape[dim]) {
            return this->emitOpError("inferred input/output operand #")
                   << opOperand.getOperandNumber() << " has shape's dimension #"
                   << dim << " to be " << inferredDimSize << ", but found "
                   << shape[dim];
          }
        } else {
          if (inferredDimSize > shape[dim]) {
            return this->emitOpError("inferred input/output operand #")
                   << opOperand.getOperandNumber() << " has shape's dimension #"
                   << dim << " to be greater than or equal to "
                   << inferredDimSize << ", but found " << shape[dim];
          }
        }
      }
    }
  }

  return success();
}
