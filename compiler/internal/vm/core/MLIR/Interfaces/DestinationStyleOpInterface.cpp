//===- DestinationStyleOpInterface.cpp -- Destination style ops -----------===//
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

#include "mlir/Interfaces/DestinationStyleOpInterface.h"

using namespace mlir;

namespace mlir {
#include "mlir/Interfaces/DestinationStyleOpInterface.cpp.inc"
} // namespace mlir

namespace {
size_t getNumTensorResults(Operation *op) {
  size_t numTensorResults = 0;
  for (auto t : op->getResultTypes()) {
    if (isa<TensorType>(t)) {
      ++numTensorResults;
    }
  }
  return numTensorResults;
}
} // namespace

LogicalResult detail::verifyDestinationStyleOpInterface(Operation *op) {
  DestinationStyleOpInterface dstStyleOp =
      cast<DestinationStyleOpInterface>(op);

  SmallVector<OpOperand *> outputTensorOperands;
  for (OpOperand &operand : dstStyleOp.getDpsInitsMutable()) {
    Type type = operand.get().getType();
    if (isa<TensorType>(type)) {
      outputTensorOperands.push_back(&operand);
    } else if (!isa<BaseMemRefType>(type)) {
      return op->emitOpError("expected that operand #")
             << operand.getOperandNumber() << " is a tensor or a memref";
    }
  }

  // Verify the number of tensor results matches the number of output tensors.
  if (getNumTensorResults(op) != outputTensorOperands.size())
    return op->emitOpError("expected the number of tensor results (")
           << getNumTensorResults(op)
           << ") to be equal to the number of output tensors ("
           << outputTensorOperands.size() << ")";

  for (OpOperand *opOperand : outputTensorOperands) {
    OpResult result = dstStyleOp.getTiedOpResult(opOperand);
    if (result.getType() != opOperand->get().getType())
      return op->emitOpError("expected type of operand #")
             << opOperand->getOperandNumber() << " ("
             << opOperand->get().getType() << ")"
             << " to match type of corresponding result (" << result.getType()
             << ")";
  }

  return success();
}
