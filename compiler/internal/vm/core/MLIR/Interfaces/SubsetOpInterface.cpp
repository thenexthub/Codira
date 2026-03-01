//===- SubsetOpInterface.cpp - Tensor Subsets -----------------------------===//
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

#include "mlir/Interfaces/SubsetOpInterface.h"
#include "mlir/Interfaces/DestinationStyleOpInterface.h"
#include "mlir/Interfaces/ValueBoundsOpInterface.h"

#include "mlir/Interfaces/SubsetOpInterface.cpp.inc"

using namespace mlir;

OpOperand &detail::defaultGetDestinationOperand(Operation *op) {
  auto dstOp = dyn_cast<DestinationStyleOpInterface>(op);
  assert(dstOp && "getDestination must be implemented for non-DPS ops");
  assert(
      dstOp.getNumDpsInits() == 1 &&
      "getDestination must be implemented for ops with 0 or more than 1 init");
  return *dstOp.getDpsInitOperand(0);
}

OpResult detail::defaultGetUpdatedDestination(Operation *op) {
  auto dstOp = dyn_cast<DestinationStyleOpInterface>(op);
  assert(dstOp && "getUpdatedDestination must be implemented for non-DPS ops");
  auto insertionOp = cast<SubsetInsertionOpInterface>(op);
  return dstOp.getTiedOpResult(&insertionOp.getDestinationOperand());
}

bool detail::defaultIsEquivalentSubset(
    Operation *op, Value candidate,
    function_ref<bool(Value, Value)> equivalenceFn) {
  assert(isa<SubsetInsertionOpInterface>(op) &&
         "expected SubsetInsertionOpInterface");
  if (!candidate.getDefiningOp<SubsetExtractionOpInterface>())
    return false;
  return cast<SubsetOpInterface>(op).operatesOnEquivalentSubset(
      candidate.getDefiningOp<SubsetOpInterface>(), equivalenceFn);
}

bool detail::defaultOperatesOnEquivalentSubset(
    Operation *op, SubsetOpInterface candidate,
    function_ref<bool(Value, Value)> equivalenceFn) {
  auto subsetOp = cast<SubsetOpInterface>(op);
  FailureOr<HyperrectangularSlice> slice =
      subsetOp.getAccessedHyperrectangularSlice();
  assert(succeeded(slice) &&
         "operatesOnEquivalentSubset must be implemented if "
         "getAccessedHyperrectangularSlice is not implemented");
  FailureOr<HyperrectangularSlice> otherSlice =
      candidate.getAccessedHyperrectangularSlice();
  if (failed(otherSlice))
    return false;
  if (!equivalenceFn(subsetOp.getTensorContainer(),
                     candidate.getTensorContainer()))
    return false;
  FailureOr<bool> equivalent = ValueBoundsConstraintSet::areEquivalentSlices(
      op->getContext(), *slice, *otherSlice);
  return succeeded(equivalent) && *equivalent;
}

bool detail::defaultOperatesOnDisjointSubset(
    Operation *op, SubsetOpInterface candidate,
    function_ref<bool(Value, Value)> equivalenceFn) {
  auto subsetOp = cast<SubsetOpInterface>(op);
  FailureOr<HyperrectangularSlice> slice =
      subsetOp.getAccessedHyperrectangularSlice();
  assert(succeeded(slice) &&
         "defaultOperatesOnDisjointSubset must be implemented if "
         "getAccessedHyperrectangularSlice is not implemented");
  FailureOr<HyperrectangularSlice> otherSlice =
      candidate.getAccessedHyperrectangularSlice();
  if (failed(otherSlice))
    return false;
  if (!equivalenceFn(subsetOp.getTensorContainer(),
                     candidate.getTensorContainer()))
    return false;
  FailureOr<bool> overlapping = ValueBoundsConstraintSet::areOverlappingSlices(
      op->getContext(), *slice, *otherSlice);
  return succeeded(overlapping) && !*overlapping;
}

Value detail::getTensorContainer(Operation *op) {
  if (auto insertionOp = dyn_cast<::mlir::SubsetInsertionOpInterface>(op))
    return insertionOp.getDestinationOperand().get();
  return cast<::mlir::SubsetExtractionOpInterface>(op).getSourceOperand().get();
}

LogicalResult detail::verifySubsetOpInterface(SubsetOpInterface op) {
  if (!(isa<SubsetExtractionOpInterface>(op.getOperation()) ^
        isa<SubsetInsertionOpInterface>(op.getOperation())))
    return op->emitOpError(
        "SubsetOpInterface ops must implement either "
        "SubsetExtractionOpInterface or SubsetInsertionOpInterface");
  return success();
}

LogicalResult
detail::verifySubsetExtractionOpInterface(SubsetExtractionOpInterface op) {
  if (op->getNumResults() != 1)
    return op->emitOpError(
        "SubsetExtractionOpInterface ops must have one result");
  return success();
}
