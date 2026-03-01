//===- ForallToFor.cpp - scf.forall to scf.for loop conversion ------------===//
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
// Transforms SCF.ForallOp's into SCF.ForOp's.
//
//===----------------------------------------------------------------------===//

#include "mlir/Dialect/SCF/Transforms/Passes.h"

#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/Dialect/SCF/Transforms/Transforms.h"
#include "mlir/IR/PatternMatch.h"

namespace mlir {
#define GEN_PASS_DEF_SCFFORALLTOFORLOOP
#include "mlir/Dialect/SCF/Transforms/Passes.h.inc"
} // namespace mlir

using namespace mlir;
using scf::LoopNest;

LogicalResult
mlir::scf::forallToForLoop(RewriterBase &rewriter, scf::ForallOp forallOp,
                           SmallVectorImpl<Operation *> *results) {
  OpBuilder::InsertionGuard guard(rewriter);
  rewriter.setInsertionPoint(forallOp);

  Location loc = forallOp.getLoc();
  SmallVector<Value> lbs = forallOp.getLowerBound(rewriter);
  SmallVector<Value> ubs = forallOp.getUpperBound(rewriter);
  SmallVector<Value> steps = forallOp.getStep(rewriter);
  LoopNest loopNest = scf::buildLoopNest(rewriter, loc, lbs, ubs, steps);

  SmallVector<Value> ivs = toolchain::map_to_vector(
      loopNest.loops, [](scf::ForOp loop) { return loop.getInductionVar(); });

  Block *innermostBlock = loopNest.loops.back().getBody();
  rewriter.eraseOp(forallOp.getBody()->getTerminator());
  rewriter.inlineBlockBefore(forallOp.getBody(), innermostBlock,
                             innermostBlock->getTerminator()->getIterator(),
                             ivs);
  rewriter.eraseOp(forallOp);

  if (results) {
    toolchain::move(loopNest.loops, std::back_inserter(*results));
  }

  return success();
}

namespace {
struct ForallToForLoop : public impl::SCFForallToForLoopBase<ForallToForLoop> {
  void runOnOperation() override {
    Operation *parentOp = getOperation();
    IRRewriter rewriter(parentOp->getContext());

    parentOp->walk([&](scf::ForallOp forallOp) {
      if (failed(scf::forallToForLoop(rewriter, forallOp))) {
        return signalPassFailure();
      }
    });
  }
};
} // namespace

std::unique_ptr<Pass> mlir::createForallToForLoopPass() {
  return std::make_unique<ForallToForLoop>();
}
