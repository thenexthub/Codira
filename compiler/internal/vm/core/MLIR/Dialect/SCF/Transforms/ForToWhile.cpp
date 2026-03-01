//===- ForToWhile.cpp - scf.for to scf.while loop conversion --------------===//
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
// Transforms SCF.ForOp's into SCF.WhileOp's.
//
//===----------------------------------------------------------------------===//

#include "mlir/Dialect/SCF/Transforms/Passes.h"

#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/Dialect/SCF/Transforms/Transforms.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"

namespace mlir {
#define GEN_PASS_DEF_SCFFORTOWHILELOOP
#include "mlir/Dialect/SCF/Transforms/Passes.h.inc"
} // namespace mlir

using namespace mlir;
using scf::ForOp;
using scf::WhileOp;

namespace {

struct ForLoopLoweringPattern : public OpRewritePattern<ForOp> {
  using OpRewritePattern<ForOp>::OpRewritePattern;

  LogicalResult matchAndRewrite(ForOp forOp,
                                PatternRewriter &rewriter) const override {
    // Generate type signature for the loop-carried values. The induction
    // variable is placed first, followed by the forOp.iterArgs.
    SmallVector<Type> lcvTypes;
    SmallVector<Location> lcvLocs;
    lcvTypes.push_back(forOp.getInductionVar().getType());
    lcvLocs.push_back(forOp.getInductionVar().getLoc());
    for (Value value : forOp.getInitArgs()) {
      lcvTypes.push_back(value.getType());
      lcvLocs.push_back(value.getLoc());
    }

    // Build scf.WhileOp
    SmallVector<Value> initArgs;
    initArgs.push_back(forOp.getLowerBound());
    toolchain::append_range(initArgs, forOp.getInitArgs());
    auto whileOp = WhileOp::create(rewriter, forOp.getLoc(), lcvTypes, initArgs,
                                   forOp->getAttrs());

    // 'before' region contains the loop condition and forwarding of iteration
    // arguments to the 'after' region.
    auto *beforeBlock = rewriter.createBlock(
        &whileOp.getBefore(), whileOp.getBefore().begin(), lcvTypes, lcvLocs);
    rewriter.setInsertionPointToStart(whileOp.getBeforeBody());
    arith::CmpIPredicate predicate = forOp.getUnsignedCmp()
                                         ? arith::CmpIPredicate::ult
                                         : arith::CmpIPredicate::slt;
    auto cmpOp = arith::CmpIOp::create(rewriter, whileOp.getLoc(), predicate,
                                       beforeBlock->getArgument(0),
                                       forOp.getUpperBound());
    scf::ConditionOp::create(rewriter, whileOp.getLoc(), cmpOp.getResult(),
                             beforeBlock->getArguments());

    // Inline for-loop body into an executeRegion operation in the "after"
    // region. The return type of the execRegionOp does not contain the
    // iv - yields in the source for-loop contain only iterArgs.
    auto *afterBlock = rewriter.createBlock(
        &whileOp.getAfter(), whileOp.getAfter().begin(), lcvTypes, lcvLocs);

    // Add induction variable incrementation
    rewriter.setInsertionPointToEnd(afterBlock);
    auto ivIncOp =
        arith::AddIOp::create(rewriter, whileOp.getLoc(),
                              afterBlock->getArgument(0), forOp.getStep());

    // Rewrite uses of the for-loop block arguments to the new while-loop
    // "after" arguments
    for (const auto &barg : enumerate(forOp.getBody(0)->getArguments()))
      rewriter.replaceAllUsesWith(barg.value(),
                                  afterBlock->getArgument(barg.index()));

    // Inline for-loop body operations into 'after' region.
    for (auto &arg : toolchain::make_early_inc_range(*forOp.getBody()))
      rewriter.moveOpBefore(&arg, afterBlock, afterBlock->end());

    // Add incremented IV to yield operations
    for (auto yieldOp : afterBlock->getOps<scf::YieldOp>()) {
      SmallVector<Value> yieldOperands = yieldOp.getOperands();
      yieldOperands.insert(yieldOperands.begin(), ivIncOp.getResult());
      rewriter.modifyOpInPlace(yieldOp,
                               [&]() { yieldOp->setOperands(yieldOperands); });
    }

    // We cannot do a direct replacement of the forOp since the while op returns
    // an extra value (the induction variable escapes the loop through being
    // carried in the set of iterargs). Instead, rewrite uses of the forOp
    // results.
    for (const auto &arg : toolchain::enumerate(forOp.getResults()))
      rewriter.replaceAllUsesWith(arg.value(),
                                  whileOp.getResult(arg.index() + 1));

    rewriter.eraseOp(forOp);
    return success();
  }
};

struct ForToWhileLoop : public impl::SCFForToWhileLoopBase<ForToWhileLoop> {
  void runOnOperation() override {
    auto *parentOp = getOperation();
    MLIRContext *ctx = parentOp->getContext();
    RewritePatternSet patterns(ctx);
    patterns.add<ForLoopLoweringPattern>(ctx);
    (void)applyPatternsGreedily(parentOp, std::move(patterns));
  }
};
} // namespace

std::unique_ptr<Pass> mlir::createForToWhileLoopPass() {
  return std::make_unique<ForToWhileLoop>();
}
