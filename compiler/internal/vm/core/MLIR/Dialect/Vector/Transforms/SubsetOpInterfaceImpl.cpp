//===- SubsetOpInterfaceImpl.cpp - Tensor subsets -------------------------===//
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

#include "mlir/Dialect/Vector/Transforms/SubsetOpInterfaceImpl.h"

#include "mlir/Dialect/Vector/IR/VectorOps.h"
#include "mlir/Interfaces/SubsetOpInterface.h"

using namespace mlir;
using namespace mlir::vector;

namespace {

template <typename OpTy>
struct XferOpSubsetOpInterface
    : public SubsetOpInterface::ExternalModel<XferOpSubsetOpInterface<OpTy>,
                                              OpTy> {
  FailureOr<HyperrectangularSlice>
  getAccessedHyperrectangularSlice(Operation *op) const {
    auto xferOp = cast<OpTy>(op);
    Builder b(xferOp->getContext());
    SmallVector<OpFoldResult> offsets = toolchain::map_to_vector(
        xferOp.getIndices(), [](Value v) -> OpFoldResult { return v; });
    SmallVector<OpFoldResult> sizes = toolchain::map_to_vector(
        xferOp.getTransferChunkAccessed(),
        [&](int64_t sz) -> OpFoldResult { return b.getIndexAttr(sz); });
    return HyperrectangularSlice(offsets, sizes);
  }
};

struct TransferReadOpSubsetExtractionOpInterface
    : public SubsetExtractionOpInterface::ExternalModel<
          TransferReadOpSubsetExtractionOpInterface, vector::TransferReadOp> {
  OpOperand &getSourceOperand(Operation *op) const {
    return cast<vector::TransferReadOp>(op).getBaseMutable();
  }
};

struct TransferWriteOpSubsetInsertionOpInterface
    : public SubsetInsertionOpInterface::ExternalModel<
          TransferWriteOpSubsetInsertionOpInterface, vector::TransferWriteOp> {
  OpOperand &getSourceOperand(Operation *op) const {
    return cast<vector::TransferWriteOp>(op).getValueToStoreMutable();
  }

  OpOperand &getDestinationOperand(Operation *op) const {
    return cast<vector::TransferWriteOp>(op).getBaseMutable();
  }

  Value buildSubsetExtraction(Operation *op, OpBuilder &builder,
                              Location loc) const {
    // TODO: Implement when needed.
    return Value();
  }

  SmallVector<Value>
  getValuesNeededToBuildSubsetExtraction(Operation *op) const {
    // TODO: Implement when needed.
    return {};
  }
};

} // namespace

void mlir::vector::registerSubsetOpInterfaceExternalModels(
    DialectRegistry &registry) {
  registry.addExtension(+[](MLIRContext *ctx, vector::VectorDialect *dialect) {
    TransferReadOp::attachInterface<XferOpSubsetOpInterface<TransferReadOp>>(
        *ctx);
    TransferReadOp::attachInterface<TransferReadOpSubsetExtractionOpInterface>(
        *ctx);
    TransferWriteOp::attachInterface<XferOpSubsetOpInterface<TransferWriteOp>>(
        *ctx);
    TransferWriteOp::attachInterface<TransferWriteOpSubsetInsertionOpInterface>(
        *ctx);
  });
}
