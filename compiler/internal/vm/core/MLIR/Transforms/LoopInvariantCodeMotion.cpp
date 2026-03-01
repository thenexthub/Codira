//===- LoopInvariantCodeMotion.cpp - Code to perform loop fusion-----------===//
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
// This file implements loop invariant code motion.
//
//===----------------------------------------------------------------------===//

#include "mlir/Transforms/Passes.h"

#include "mlir/IR/PatternMatch.h"
#include "mlir/Interfaces/LoopLikeInterface.h"
#include "mlir/Transforms/LoopInvariantCodeMotionUtils.h"

namespace mlir {
#define GEN_PASS_DEF_LOOPINVARIANTCODEMOTION
#define GEN_PASS_DEF_LOOPINVARIANTSUBSETHOISTING
#include "mlir/Transforms/Passes.h.inc"
} // namespace mlir

using namespace mlir;

namespace {
/// Loop invariant code motion (LICM) pass.
struct LoopInvariantCodeMotion
    : public impl::LoopInvariantCodeMotionBase<LoopInvariantCodeMotion> {
  void runOnOperation() override;
};

struct LoopInvariantSubsetHoisting
    : public impl::LoopInvariantSubsetHoistingBase<
          LoopInvariantSubsetHoisting> {
  void runOnOperation() override;
};
} // namespace

void LoopInvariantCodeMotion::runOnOperation() {
  // Walk through all loops in a function in innermost-loop-first order. This
  // way, we first LICM from the inner loop, and place the ops in
  // the outer loop, which in turn can be further LICM'ed.
  getOperation()->walk(
      [&](LoopLikeOpInterface loopLike) { moveLoopInvariantCode(loopLike); });
}

void LoopInvariantSubsetHoisting::runOnOperation() {
  IRRewriter rewriter(getOperation()->getContext());
  // Walk through all loops in a function in innermost-loop-first order. This
  // way, we first hoist from the inner loop, and place the ops in the outer
  // loop, which in turn can be further hoisted from.
  getOperation()->walk([&](LoopLikeOpInterface loopLike) {
    (void)hoistLoopInvariantSubsets(rewriter, loopLike);
  });
}

std::unique_ptr<Pass> mlir::createLoopInvariantCodeMotionPass() {
  return std::make_unique<LoopInvariantCodeMotion>();
}

std::unique_ptr<Pass> mlir::createLoopInvariantSubsetHoistingPass() {
  return std::make_unique<LoopInvariantSubsetHoisting>();
}
