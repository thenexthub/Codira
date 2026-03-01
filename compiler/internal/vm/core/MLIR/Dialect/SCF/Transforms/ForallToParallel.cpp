//===- ForallToParallel.cpp - scf.forall to scf.parallel loop conversion --===//
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
// Transforms SCF.ForallOp's into SCF.ParallelOps's.
//
//===----------------------------------------------------------------------===//

#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/Dialect/SCF/Transforms/Passes.h"
#include "mlir/Dialect/SCF/Transforms/Transforms.h"
#include "mlir/IR/PatternMatch.h"

namespace mlir {
#define GEN_PASS_DEF_SCFFORALLTOPARALLELLOOP
#include "mlir/Dialect/SCF/Transforms/Passes.h.inc"
} // namespace mlir

using namespace mlir;

LogicalResult mlir::scf::forallToParallelLoop(RewriterBase &rewriter,
                                              scf::ForallOp forallOp,
                                              scf::ParallelOp *result) {
  OpBuilder::InsertionGuard guard(rewriter);
  rewriter.setInsertionPoint(forallOp);

  Location loc = forallOp.getLoc();
  if (!forallOp.getOutputs().empty())
    return rewriter.notifyMatchFailure(
        forallOp,
        "only fully bufferized scf.forall ops can be lowered to scf.parallel");

  // Convert mixed bounds and steps to SSA values.
  SmallVector<Value> lbs = forallOp.getLowerBound(rewriter);
  SmallVector<Value> ubs = forallOp.getUpperBound(rewriter);
  SmallVector<Value> steps = forallOp.getStep(rewriter);

  // Create empty scf.parallel op.
  auto parallelOp = scf::ParallelOp::create(rewriter, loc, lbs, ubs, steps);
  rewriter.eraseBlock(&parallelOp.getRegion().front());
  rewriter.inlineRegionBefore(forallOp.getRegion(), parallelOp.getRegion(),
                              parallelOp.getRegion().begin());
  // Replace the terminator.
  rewriter.setInsertionPointToEnd(&parallelOp.getRegion().front());
  rewriter.replaceOpWithNewOp<scf::ReduceOp>(
      parallelOp.getRegion().front().getTerminator());

  // If the mapping attribute is present, propagate to the new parallelOp.
  if (forallOp.getMapping())
    parallelOp->setAttr("mapping", *forallOp.getMapping());

  // Erase the scf.forall op.
  rewriter.replaceOp(forallOp, parallelOp);

  if (result)
    *result = parallelOp;

  return success();
}

namespace {
struct ForallToParallelLoop final
    : public impl::SCFForallToParallelLoopBase<ForallToParallelLoop> {
  void runOnOperation() override {
    Operation *parentOp = getOperation();
    IRRewriter rewriter(parentOp->getContext());

    parentOp->walk([&](scf::ForallOp forallOp) {
      if (failed(scf::forallToParallelLoop(rewriter, forallOp))) {
        return signalPassFailure();
      }
    });
  }
};
} // namespace

std::unique_ptr<Pass> mlir::createForallToParallelLoopPass() {
  return std::make_unique<ForallToParallelLoop>();
}
