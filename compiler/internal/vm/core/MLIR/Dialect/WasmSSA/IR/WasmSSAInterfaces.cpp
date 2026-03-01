//===- WasmSSAInterfaces.cpp - WasmSSA Interfaces -*- C++ -*-===//
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
// This file defines op interfaces for the WasmSSA dialect in MLIR.
//
//===----------------------------------------------------------------------===//

#include "mlir/Dialect/WasmSSA/IR/WasmSSAInterfaces.h"
#include "mlir/Dialect/WasmSSA/IR/WasmSSA.h"
#include "mlir/IR/Operation.h"
#include "mlir/IR/Visitors.h"
#include "mlir/Support/LLVM.h"
#include "vm/core/Support/LogicalResult.h"

namespace mlir::wasmssa {
#include "mlir/Dialect/WasmSSA/IR/WasmSSAInterfaces.cpp.inc"

namespace detail {
LogicalResult verifyLabelBranchingOpInterface(Operation *op) {
  auto branchInterface = dyn_cast<LabelBranchingOpInterface>(op);
  toolchain::FailureOr<LabelLevelOpInterface> res =
      LabelBranchingOpInterface::getTargetOpFromBlock(
          op->getBlock(), branchInterface.getExitLevel());
  return res;
}

LogicalResult verifyConstantExpressionInterface(Operation *op) {
  Region &initializerRegion = op->getRegion(0);
  WalkResult resultState =
      initializerRegion.walk([&](Operation *currentOp) -> WalkResult {
        if (isa<ReturnOp>(currentOp) ||
            currentOp->hasTrait<ConstantExprOpTrait>())
          return WalkResult::advance();
        op->emitError("expected a constant initializer for this operator, got ")
            << currentOp;
        return WalkResult::interrupt();
      });
  return success(!resultState.wasInterrupted());
}

LogicalResult verifyLabelLevelInterface(Operation *op) {
  Block *target = cast<LabelLevelOpInterface>(op).getLabelTarget();
  Region *targetRegion = target->getParent();
  if (targetRegion != op->getParentRegion() &&
      targetRegion->getParentOp() != op)
    return op->emitError("target should be a block defined in same level than "
                         "operation or in its region.");
  return success();
}
} // namespace detail

toolchain::FailureOr<LabelLevelOpInterface>
LabelBranchingOpInterface::getTargetOpFromBlock(::mlir::Block *block,
                                                uint32_t breakLevel) {
  LabelLevelOpInterface res{};
  for (size_t curLevel{0}; curLevel <= breakLevel; curLevel++) {
    res = dyn_cast_or_null<LabelLevelOpInterface>(block->getParentOp());
    if (!res)
      return failure();
    block = res->getBlock();
  }
  return res;
}
} // namespace mlir::wasmssa
