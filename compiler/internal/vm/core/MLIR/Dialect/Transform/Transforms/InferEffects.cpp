//===- InferEffects.cpp - Infer memory effects for named symbols ----------===//
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

#include "mlir/Dialect/Transform/IR/TransformDialect.h"
#include "mlir/Dialect/Transform/Transforms/Passes.h"

#include "mlir/Dialect/Transform/Interfaces/TransformInterfaces.h"
#include "mlir/IR/Visitors.h"
#include "mlir/Interfaces/FunctionInterfaces.h"
#include "vm/core/ADT/DenseSet.h"

using namespace mlir;

namespace mlir {
namespace transform {
#define GEN_PASS_DEF_INFEREFFECTSPASS
#include "mlir/Dialect/Transform/Transforms/Passes.h.inc"
} // namespace transform
} // namespace mlir

static LogicalResult inferSideEffectAnnotations(Operation *op) {
  if (!isa<transform::TransformOpInterface>(op))
    return success();

  auto func = dyn_cast<FunctionOpInterface>(op);
  if (!func || func.isExternal())
    return success();

  if (!func.getFunctionBody().hasOneBlock()) {
    return op->emitError()
           << "only single-block operations are currently supported";
  }

  // Note that there can't be an inclusion of an unannotated symbol because it
  // wouldn't have passed the verifier, so recursion isn't necessary here.
  toolchain::SmallDenseSet<unsigned> consumedArguments;
  transform::getConsumedBlockArguments(func.getFunctionBody().front(),
                                       consumedArguments);

  for (unsigned i = 0, e = func.getNumArguments(); i < e; ++i) {
    func.setArgAttr(i,
                    consumedArguments.contains(i)
                        ? transform::TransformDialect::kArgConsumedAttrName
                        : transform::TransformDialect::kArgReadOnlyAttrName,
                    UnitAttr::get(op->getContext()));
  }
  return success();
}

namespace {
class InferEffectsPass
    : public transform::impl::InferEffectsPassBase<InferEffectsPass> {
public:
  void runOnOperation() override {
    WalkResult result = getOperation()->walk([](Operation *op) {
      return failed(inferSideEffectAnnotations(op)) ? WalkResult::interrupt()
                                                    : WalkResult::advance();
    });
    if (result.wasInterrupted())
      return signalPassFailure();
  }
};
} // namespace
