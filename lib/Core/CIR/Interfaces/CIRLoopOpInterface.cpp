//===---------------------------------------------------------------------===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
// 
// Author: Tunjay Akbarli
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
// 
//     http://www.apache.org/licenses/LICENSE-2.0
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
//===---------------------------------------------------------------------===//

#include "language/Core/CIR/Interfaces/CIRLoopOpInterface.h"

#include "language/Core/CIR/Dialect/IR/CIRDialect.h"
#include "language/Core/CIR/Interfaces/CIRLoopOpInterface.cpp.inc"
#include "toolchain/Support/ErrorHandling.h"

namespace cir {

void LoopOpInterface::getLoopOpSuccessorRegions(
    LoopOpInterface op, mlir::RegionBranchPoint point,
    toolchain::SmallVectorImpl<mlir::RegionSuccessor> &regions) {
  assert(point.isParent() || point.getRegionOrNull());

  // Branching to first region: go to condition or body (do-while).
  if (point.isParent()) {
    regions.emplace_back(&op.getEntry(), op.getEntry().getArguments());
    return;
  }

  // Branching from condition: go to body or exit.
  if (&op.getCond() == point.getRegionOrNull()) {
    regions.emplace_back(mlir::RegionSuccessor(op->getResults()));
    regions.emplace_back(&op.getBody(), op.getBody().getArguments());
    return;
  }

  // Branching from body: go to step (for) or condition.
  if (&op.getBody() == point.getRegionOrNull()) {
    // FIXME(cir): Should we consider break/continue statements here?
    mlir::Region *afterBody =
        (op.maybeGetStep() ? op.maybeGetStep() : &op.getCond());
    regions.emplace_back(afterBody, afterBody->getArguments());
    return;
  }

  // Branching from step: go to condition.
  if (op.maybeGetStep() == point.getRegionOrNull()) {
    regions.emplace_back(&op.getCond(), op.getCond().getArguments());
    return;
  }

  toolchain_unreachable("unexpected branch origin");
}

/// Verify invariants of the LoopOpInterface.
toolchain::LogicalResult detail::verifyLoopOpInterface(mlir::Operation *op) {
  // FIXME: fix this so the conditionop isn't requiring MLIRCIR
  // auto loopOp = mlir::cast<LoopOpInterface>(op);
  // if (!mlir::isa<ConditionOp>(loopOp.getCond().back().getTerminator()))
  //   return op->emitOpError(
  //       "expected condition region to terminate with 'cir.condition'");
  return toolchain::success();
}

} // namespace cir
