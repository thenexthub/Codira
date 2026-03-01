//===- ControlFlowSink.cpp - Code to perform control-flow sinking ---------===//
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
// This file implements a basic control-flow sink pass. Control-flow sinking
// moves operations whose only uses are in conditionally-executed blocks in to
// those blocks so that they aren't executed on paths where their results are
// not needed.
//
//===----------------------------------------------------------------------===//

#include "mlir/Transforms/Passes.h"

#include "mlir/IR/Dominance.h"
#include "mlir/Interfaces/ControlFlowInterfaces.h"
#include "mlir/Interfaces/SideEffectInterfaces.h"
#include "mlir/Transforms/ControlFlowSinkUtils.h"

namespace mlir {
#define GEN_PASS_DEF_CONTROLFLOWSINK
#include "mlir/Transforms/Passes.h.inc"
} // namespace mlir

using namespace mlir;

namespace {
/// A control-flow sink pass.
struct ControlFlowSink : public impl::ControlFlowSinkBase<ControlFlowSink> {
  void runOnOperation() override;
};
} // end anonymous namespace

void ControlFlowSink::runOnOperation() {
  auto &domInfo = getAnalysis<DominanceInfo>();
  getOperation()->walk([&](RegionBranchOpInterface branch) {
    SmallVector<Region *> regionsToSink;
    // Get the regions are that known to be executed at most once.
    getSinglyExecutedRegionsToSink(branch, regionsToSink);
    // Sink side-effect free operations.
    numSunk = controlFlowSink(
        regionsToSink, domInfo,
        [](Operation *op, Region *) { return isMemoryEffectFree(op); },
        [](Operation *op, Region *region) {
          // Move the operation to the beginning of the region's entry block.
          // This guarantees the preservation of SSA dominance of all of the
          // operation's uses are in the region.
          op->moveBefore(&region->front(), region->front().begin());
        });
  });
}

std::unique_ptr<Pass> mlir::createControlFlowSinkPass() {
  return std::make_unique<ControlFlowSink>();
}
