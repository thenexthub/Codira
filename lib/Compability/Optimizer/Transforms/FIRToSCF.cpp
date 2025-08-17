//===-- FIRToSCF.cpp ------------------------------------------------------===//
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
//===----------------------------------------------------------------------===//

#include "language/Compability/Optimizer/Dialect/FIRDialect.h"
#include "language/Compability/Optimizer/Transforms/Passes.h"
#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/Transforms/DialectConversion.h"

namespace fir {
#define GEN_PASS_DEF_FIRTOSCFPASS
#include "language/Compability/Optimizer/Transforms/Passes.h.inc"
} // namespace fir

namespace {
class FIRToSCFPass : public fir::impl::FIRToSCFPassBase<FIRToSCFPass> {
public:
  void runOnOperation() override;
};

struct DoLoopConversion : public mlir::OpRewritePattern<fir::DoLoopOp> {
  using OpRewritePattern<fir::DoLoopOp>::OpRewritePattern;

  mlir::LogicalResult
  matchAndRewrite(fir::DoLoopOp doLoopOp,
                  mlir::PatternRewriter &rewriter) const override {
    mlir::Location loc = doLoopOp.getLoc();
    bool hasFinalValue = doLoopOp.getFinalValue().has_value();

    // Get loop values from the DoLoopOp
    mlir::Value low = doLoopOp.getLowerBound();
    mlir::Value high = doLoopOp.getUpperBound();
    assert(low && high && "must be a Value");
    mlir::Value step = doLoopOp.getStep();
    mlir::SmallVector<mlir::Value> iterArgs;
    if (hasFinalValue)
      iterArgs.push_back(low);
    iterArgs.append(doLoopOp.getIterOperands().begin(),
                    doLoopOp.getIterOperands().end());

    // fir.do_loop iterates over the interval [%l, %u], and the step may be
    // negative. But scf.for iterates over the interval [%l, %u), and the step
    // must be a positive value.
    // For easier conversion, we calculate the trip count and use a canonical
    // induction variable.
    auto diff = mlir::arith::SubIOp::create(rewriter, loc, high, low);
    auto distance = mlir::arith::AddIOp::create(rewriter, loc, diff, step);
    auto tripCount =
        mlir::arith::DivSIOp::create(rewriter, loc, distance, step);
    auto zero = mlir::arith::ConstantIndexOp::create(rewriter, loc, 0);
    auto one = mlir::arith::ConstantIndexOp::create(rewriter, loc, 1);
    auto scfForOp =
        mlir::scf::ForOp::create(rewriter, loc, zero, tripCount, one, iterArgs);

    auto &loopOps = doLoopOp.getBody()->getOperations();
    auto resultOp =
        mlir::cast<fir::ResultOp>(doLoopOp.getBody()->getTerminator());
    auto results = resultOp.getOperands();
    mlir::Block *loweredBody = scfForOp.getBody();

    loweredBody->getOperations().splice(loweredBody->begin(), loopOps,
                                        loopOps.begin(),
                                        std::prev(loopOps.end()));

    rewriter.setInsertionPointToStart(loweredBody);
    mlir::Value iv = mlir::arith::MulIOp::create(
        rewriter, loc, scfForOp.getInductionVar(), step);
    iv = mlir::arith::AddIOp::create(rewriter, loc, low, iv);

    if (!results.empty()) {
      rewriter.setInsertionPointToEnd(loweredBody);
      mlir::scf::YieldOp::create(rewriter, resultOp->getLoc(), results);
    }
    doLoopOp.getInductionVar().replaceAllUsesWith(iv);
    rewriter.replaceAllUsesWith(doLoopOp.getRegionIterArgs(),
                                hasFinalValue
                                    ? scfForOp.getRegionIterArgs().drop_front()
                                    : scfForOp.getRegionIterArgs());

    // Copy all the attributes from the old to new op.
    scfForOp->setAttrs(doLoopOp->getAttrs());
    rewriter.replaceOp(doLoopOp, scfForOp);
    return mlir::success();
  }
};

struct IterWhileConversion : public mlir::OpRewritePattern<fir::IterWhileOp> {
  using OpRewritePattern<fir::IterWhileOp>::OpRewritePattern;

  mlir::LogicalResult
  matchAndRewrite(fir::IterWhileOp iterWhileOp,
                  mlir::PatternRewriter &rewriter) const override {

    mlir::Location loc = iterWhileOp.getLoc();
    mlir::Value lowerBound = iterWhileOp.getLowerBound();
    mlir::Value upperBound = iterWhileOp.getUpperBound();
    mlir::Value step = iterWhileOp.getStep();

    mlir::Value okInit = iterWhileOp.getIterateIn();
    mlir::ValueRange iterArgs = iterWhileOp.getInitArgs();

    mlir::SmallVector<mlir::Value> initVals;
    initVals.push_back(lowerBound);
    initVals.push_back(okInit);
    initVals.append(iterArgs.begin(), iterArgs.end());

    mlir::SmallVector<mlir::Type> loopTypes;
    loopTypes.push_back(lowerBound.getType());
    loopTypes.push_back(okInit.getType());
    for (auto val : iterArgs)
      loopTypes.push_back(val.getType());

    auto scfWhileOp =
        mlir::scf::WhileOp::create(rewriter, loc, loopTypes, initVals);

    auto &beforeBlock = *rewriter.createBlock(
        &scfWhileOp.getBefore(), scfWhileOp.getBefore().end(), loopTypes,
        mlir::SmallVector<mlir::Location>(loopTypes.size(), loc));

    mlir::Region::BlockArgListType argsInBefore =
        scfWhileOp.getBefore().getArguments();
    auto ivInBefore = argsInBefore[0];
    auto earlyExitInBefore = argsInBefore[1];

    rewriter.setInsertionPointToStart(&beforeBlock);

    mlir::Value inductionCmp = mlir::arith::CmpIOp::create(
        rewriter, loc, mlir::arith::CmpIPredicate::sle, ivInBefore, upperBound);
    mlir::Value cond = mlir::arith::AndIOp::create(rewriter, loc, inductionCmp,
                                                   earlyExitInBefore);

    mlir::scf::ConditionOp::create(rewriter, loc, cond, argsInBefore);

    rewriter.moveBlockBefore(iterWhileOp.getBody(), &scfWhileOp.getAfter(),
                             scfWhileOp.getAfter().begin());

    auto *afterBody = scfWhileOp.getAfterBody();
    auto resultOp = mlir::cast<fir::ResultOp>(afterBody->getTerminator());
    mlir::SmallVector<mlir::Value> results(resultOp->getOperands());
    mlir::Value ivInAfter = scfWhileOp.getAfterArguments()[0];

    rewriter.setInsertionPointToStart(afterBody);
    results[0] = mlir::arith::AddIOp::create(rewriter, loc, ivInAfter, step);

    rewriter.setInsertionPointToEnd(afterBody);
    rewriter.replaceOpWithNewOp<mlir::scf::YieldOp>(resultOp, results);

    scfWhileOp->setAttrs(iterWhileOp->getAttrs());
    rewriter.replaceOp(iterWhileOp, scfWhileOp);
    return mlir::success();
  }
};

void copyBlockAndTransformResult(mlir::PatternRewriter &rewriter,
                                 mlir::Block &srcBlock, mlir::Block &dstBlock) {
  mlir::Operation *srcTerminator = srcBlock.getTerminator();
  auto resultOp = mlir::cast<fir::ResultOp>(srcTerminator);

  dstBlock.getOperations().splice(dstBlock.begin(), srcBlock.getOperations(),
                                  srcBlock.begin(), std::prev(srcBlock.end()));

  if (!resultOp->getOperands().empty()) {
    rewriter.setInsertionPointToEnd(&dstBlock);
    mlir::scf::YieldOp::create(rewriter, resultOp->getLoc(),
                               resultOp->getOperands());
  }

  rewriter.eraseOp(srcTerminator);
}

struct IfConversion : public mlir::OpRewritePattern<fir::IfOp> {
  using OpRewritePattern<fir::IfOp>::OpRewritePattern;
  mlir::LogicalResult
  matchAndRewrite(fir::IfOp ifOp,
                  mlir::PatternRewriter &rewriter) const override {
    bool hasElse = !ifOp.getElseRegion().empty();
    auto scfIfOp =
        mlir::scf::IfOp::create(rewriter, ifOp.getLoc(), ifOp.getResultTypes(),
                                ifOp.getCondition(), hasElse);

    copyBlockAndTransformResult(rewriter, ifOp.getThenRegion().front(),
                                scfIfOp.getThenRegion().front());

    if (hasElse) {
      copyBlockAndTransformResult(rewriter, ifOp.getElseRegion().front(),
                                  scfIfOp.getElseRegion().front());
    }

    scfIfOp->setAttrs(ifOp->getAttrs());
    rewriter.replaceOp(ifOp, scfIfOp);
    return mlir::success();
  }
};
} // namespace

void FIRToSCFPass::runOnOperation() {
  mlir::RewritePatternSet patterns(&getContext());
  patterns.add<DoLoopConversion, IterWhileConversion, IfConversion>(
      patterns.getContext());
  mlir::ConversionTarget target(getContext());
  target.addIllegalOp<fir::DoLoopOp, fir::IterWhileOp, fir::IfOp>();
  target.markUnknownOpDynamicallyLegal([](mlir::Operation *) { return true; });
  if (failed(
          applyPartialConversion(getOperation(), target, std::move(patterns))))
    signalPassFailure();
}

std::unique_ptr<mlir::Pass> fir::createFIRToSCFPass() {
  return std::make_unique<FIRToSCFPass>();
}
