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

#include "mlir/Dialect/Vector/IR/ValueBoundsOpInterfaceImpl.h"

#include "mlir/Dialect/Vector/IR/ScalableValueBoundsConstraintSet.h"
#include "mlir/Dialect/Vector/IR/VectorOps.h"
#include "mlir/Interfaces/ValueBoundsOpInterface.h"

using namespace mlir;

namespace mlir::vector {
namespace {

struct VectorScaleOpInterface
    : public ValueBoundsOpInterface::ExternalModel<VectorScaleOpInterface,
                                                   VectorScaleOp> {
  void populateBoundsForIndexValue(Operation *op, Value value,
                                   ValueBoundsConstraintSet &cstr) const {
    auto *scalableCstr = dyn_cast<ScalableValueBoundsConstraintSet>(&cstr);
    if (!scalableCstr)
      return;
    auto vscaleOp = cast<VectorScaleOp>(op);
    assert(value == vscaleOp.getResult() && "invalid value");
    if (auto vscale = scalableCstr->getVscaleValue()) {
      // All copies of vscale are equivalent.
      scalableCstr->bound(value) == cstr.getExpr(vscale);
    } else {
      // We know vscale is confined to [vscaleMin, vscaleMax].
      scalableCstr->bound(value) >= scalableCstr->getVscaleMin();
      scalableCstr->bound(value) <= scalableCstr->getVscaleMax();
      scalableCstr->setVscale(vscaleOp);
    }
  }
};

} // namespace
} // namespace mlir::vector

void mlir::vector::registerValueBoundsOpInterfaceExternalModels(
    DialectRegistry &registry) {
  registry.addExtension(+[](MLIRContext *ctx, vector::VectorDialect *dialect) {
    vector::VectorScaleOp::attachInterface<vector::VectorScaleOpInterface>(
        *ctx);
  });
}
