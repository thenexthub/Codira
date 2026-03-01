//===- ValueBoundsOpInterfaceImpl.cpp - Impl. of ValueBoundsOpInterface ---===//
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

#include "mlir/Dialect/Linalg/IR/ValueBoundsOpInterfaceImpl.h"

#include "mlir/Dialect/Linalg/IR/Linalg.h"
#include "mlir/Interfaces/ValueBoundsOpInterface.h"

using namespace mlir;

namespace mlir {
namespace linalg {
namespace {

struct IndexOpInterface
    : public ValueBoundsOpInterface::ExternalModel<IndexOpInterface, IndexOp> {
  void populateBoundsForIndexValue(Operation *op, Value value,
                                   ValueBoundsConstraintSet &cstr) const {
    auto indexOp = cast<IndexOp>(op);
    auto linalgOp = indexOp->getParentOfType<LinalgOp>();
    assert(value == indexOp.getResult() && "invalid value");

    // index >= 0
    cstr.bound(value) >= 0;

    // index < dim size
    int64_t flatDimPos =
        cast<AffineDimExpr>(
            linalgOp.getShapesToLoopsMap().getResult(indexOp.getDim()))
            .getPosition();
    // Find the `flatDimPos`-th operand dimension.
    int64_t flatDimCtr = 0;
    for (Value operand : linalgOp->getOperands()) {
      assert(flatDimPos >= flatDimCtr && "invalid pos");
      auto shapedType = toolchain::cast<ShapedType>(operand.getType());
      if (flatDimPos < flatDimCtr + shapedType.getRank()) {
        cstr.bound(value) < cstr.getExpr(operand, flatDimPos - flatDimCtr);
        break;
      }
      flatDimCtr += shapedType.getRank();
    }
  }
};

} // namespace
} // namespace linalg
} // namespace mlir

void mlir::linalg::registerValueBoundsOpInterfaceExternalModels(
    DialectRegistry &registry) {
  registry.addExtension(+[](MLIRContext *ctx, linalg::LinalgDialect *dialect) {
    IndexOp::attachInterface<IndexOpInterface>(*ctx);
    // Note: ValueBoundsOpInterface implementation is not required for ops that
    // implement `DestinationStyleOpInterface` (for querying shaped OpResults).
  });
}
