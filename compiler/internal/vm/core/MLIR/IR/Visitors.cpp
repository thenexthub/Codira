//===- Visitors.cpp - MLIR Visitor Utilities ------------------------------===//
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

#include "mlir/IR/Visitors.h"
#include "mlir/IR/Operation.h"

using namespace mlir;

WalkStage::WalkStage(Operation *op)
    : numRegions(op->getNumRegions()), nextRegion(0) {}

MutableArrayRef<Region> ForwardIterator::makeIterable(Operation &range) {
  return range.getRegions();
}

void detail::walk(Operation *op,
                  function_ref<void(Operation *, const WalkStage &)> callback) {
  WalkStage stage(op);

  for (Region &region : op->getRegions()) {
    // Invoke callback on the parent op before visiting each child region.
    callback(op, stage);
    stage.advance();

    for (Block &block : region) {
      for (Operation &nestedOp : block)
        walk(&nestedOp, callback);
    }
  }

  // Invoke callback after all regions have been visited.
  callback(op, stage);
}

WalkResult detail::walk(
    Operation *op,
    function_ref<WalkResult(Operation *, const WalkStage &)> callback) {
  WalkStage stage(op);

  for (Region &region : op->getRegions()) {
    // Invoke callback on the parent op before visiting each child region.
    WalkResult result = callback(op, stage);

    if (result.wasSkipped())
      return WalkResult::advance();
    if (result.wasInterrupted())
      return WalkResult::interrupt();

    stage.advance();

    for (Block &block : region) {
      // Early increment here in the case where the operation is erased.
      for (Operation &nestedOp : toolchain::make_early_inc_range(block))
        if (walk(&nestedOp, callback).wasInterrupted())
          return WalkResult::interrupt();
    }
  }
  return callback(op, stage);
}
