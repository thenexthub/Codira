//===- BufferViewFlowOpInterfaceImpl.cpp - Buffer View Flow Analysis ------===//
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

#include "mlir/Dialect/Arith/Transforms/BufferViewFlowOpInterfaceImpl.h"

#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Bufferization/IR/BufferViewFlowOpInterface.h"

using namespace mlir;
using namespace mlir::bufferization;

namespace mlir {
namespace arith {
namespace {

struct SelectOpInterface
    : public BufferViewFlowOpInterface::ExternalModel<SelectOpInterface,
                                                      SelectOp> {
  void
  populateDependencies(Operation *op,
                       RegisterDependenciesFn registerDependenciesFn) const {
    auto selectOp = cast<SelectOp>(op);

    // Either one of the true/false value may be selected at runtime.
    registerDependenciesFn(selectOp.getTrueValue(), selectOp.getResult());
    registerDependenciesFn(selectOp.getFalseValue(), selectOp.getResult());
  }
};

} // namespace
} // namespace arith
} // namespace mlir

void arith::registerBufferViewFlowOpInterfaceExternalModels(
    DialectRegistry &registry) {
  registry.addExtension(+[](MLIRContext *ctx, arith::ArithDialect *dialect) {
    SelectOp::attachInterface<SelectOpInterface>(*ctx);
  });
}
