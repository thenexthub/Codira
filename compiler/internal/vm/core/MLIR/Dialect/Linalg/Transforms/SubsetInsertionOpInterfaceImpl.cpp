//===- SubsetInsertionOpInterfaceImpl.cpp - Tensor subsets ----------------===//
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

#include "mlir/Dialect/Linalg/Transforms/SubsetInsertionOpInterfaceImpl.h"

#include "mlir/Dialect/Linalg/IR/Linalg.h"
#include "mlir/Interfaces/SubsetOpInterface.h"

using namespace mlir;
using namespace mlir::linalg;

namespace {
struct LinalgCopyOpSubsetOpInterface
    : public SubsetOpInterface::ExternalModel<LinalgCopyOpSubsetOpInterface,
                                              linalg::CopyOp> {
  bool operatesOnEquivalentSubset(
      Operation *op, SubsetOpInterface candidate,
      function_ref<bool(Value, Value)> equivalenceFn) const {
    // linalg.copy operates on the entire destination tensor.
    if (auto otherCopyOp = dyn_cast<linalg::CopyOp>(candidate.getOperation()))
      return equivalenceFn(cast<linalg::CopyOp>(op).getOutputs()[0],
                           otherCopyOp.getOutputs()[0]);
    // In the absence of an analysis, "false" is a conservative way to implement
    // this interface.
    return false;
  }

  bool operatesOnDisjointSubset(
      Operation *op, SubsetOpInterface candidate,
      function_ref<bool(Value, Value)> equivalenceFn) const {
    // In the absence of an analysis, "false" is a conservative way to implement
    // this interface.
    return false;
  }
};

struct LinalgCopyOpInterface
    : public SubsetInsertionOpInterface::ExternalModel<LinalgCopyOpInterface,
                                                       linalg::CopyOp> {
  OpOperand &getSourceOperand(Operation *op) const {
    auto copyOp = cast<CopyOp>(op);
    return toolchain::getSingleElement(copyOp.getInputsMutable());
  }

  bool
  isEquivalentSubset(Operation *op, Value candidate,
                     function_ref<bool(Value, Value)> equivalenceFn) const {
    auto copyOp = cast<CopyOp>(op);
    return equivalenceFn(candidate,
                         toolchain::getSingleElement(copyOp.getOutputs()));
  }

  Value buildSubsetExtraction(Operation *op, OpBuilder &builder,
                              Location loc) const {
    auto copyOp = cast<CopyOp>(op);
    return toolchain::getSingleElement(copyOp.getOutputs());
  }

  SmallVector<Value>
  getValuesNeededToBuildSubsetExtraction(Operation *op) const {
    auto copyOp = cast<CopyOp>(op);
    return {toolchain::getSingleElement(copyOp.getOutputs())};
  }
};
} // namespace

void mlir::linalg::registerSubsetOpInterfaceExternalModels(
    DialectRegistry &registry) {
  registry.addExtension(+[](MLIRContext *ctx, linalg::LinalgDialect *dialect) {
    linalg::CopyOp::attachInterface<LinalgCopyOpSubsetOpInterface>(*ctx);
    linalg::CopyOp::attachInterface<LinalgCopyOpInterface>(*ctx);
  });
}
