//===- AffineLoopNormalize.cpp - AffineLoopNormalize Pass -----------------===//
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
// This file implements a normalizer for affine loop-like ops.
//
//===----------------------------------------------------------------------===//

#include "mlir/Dialect/Affine/Transforms/Passes.h"

#include "mlir/Dialect/Affine/IR/AffineOps.h"
#include "mlir/Dialect/Affine/Utils.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"

namespace mlir {
namespace affine {
#define GEN_PASS_DEF_AFFINELOOPNORMALIZE
#include "mlir/Dialect/Affine/Transforms/Passes.h.inc"
} // namespace affine
} // namespace mlir

using namespace mlir;
using namespace mlir::affine;

namespace {

/// Normalize affine.parallel ops so that lower bounds are 0 and steps are 1.
/// As currently implemented, this pass cannot fail, but it might skip over ops
/// that are already in a normalized form.
struct AffineLoopNormalizePass
    : public affine::impl::AffineLoopNormalizeBase<AffineLoopNormalizePass> {
  explicit AffineLoopNormalizePass(bool promoteSingleIter) {
    this->promoteSingleIter = promoteSingleIter;
  }

  void runOnOperation() override {
    getOperation().walk([&](Operation *op) {
      if (auto affineParallel = dyn_cast<AffineParallelOp>(op))
        normalizeAffineParallel(affineParallel);
      else if (auto affineFor = dyn_cast<AffineForOp>(op))
        (void)normalizeAffineFor(affineFor, promoteSingleIter);
    });
  }
};

} // namespace

std::unique_ptr<OperationPass<func::FuncOp>>
mlir::affine::createAffineLoopNormalizePass(bool promoteSingleIter) {
  return std::make_unique<AffineLoopNormalizePass>(promoteSingleIter);
}
