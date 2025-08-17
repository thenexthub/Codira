//===-- ControlFlowConverter.cpp ------------------------------------------===//
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
#include "language/Compability/Optimizer/Dialect/FIROps.h"
#include "language/Compability/Optimizer/Dialect/FIROpsSupport.h"
#include "language/Compability/Optimizer/Dialect/Support/FIRContext.h"
#include "language/Compability/Optimizer/Dialect/Support/KindMapping.h"
#include "language/Compability/Optimizer/Support/InternalNames.h"
#include "language/Compability/Optimizer/Support/TypeCode.h"
#include "language/Compability/Optimizer/Transforms/Passes.h"
#include "language/Compability/Runtime/derived-api.h"
#include "mlir/Dialect/Affine/IR/AffineOps.h"
#include "mlir/Dialect/ControlFlow/IR/ControlFlowOps.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/DialectConversion.h"
#include "toolchain/ADT/SmallSet.h"
#include "toolchain/Support/CommandLine.h"

namespace fir {
#define GEN_PASS_DEF_CFGCONVERSION
#include "language/Compability/Optimizer/Transforms/Passes.h.inc"
} // namespace fir

using namespace fir;
using namespace mlir;

namespace {

// Conversion of fir control ops to more primitive control-flow.
//
// FIR loops that cannot be converted to the affine dialect will remain as
// `fir.do_loop` operations.  These can be converted to control-flow operations.

/// Convert `fir.do_loop` to CFG
class CfgLoopConv : public mlir::OpRewritePattern<fir::DoLoopOp> {
public:
  using OpRewritePattern::OpRewritePattern;

  CfgLoopConv(mlir::MLIRContext *ctx, bool forceLoopToExecuteOnce, bool setNSW)
      : mlir::OpRewritePattern<fir::DoLoopOp>(ctx),
        forceLoopToExecuteOnce(forceLoopToExecuteOnce), setNSW(setNSW) {}

  toolchain::LogicalResult
  matchAndRewrite(DoLoopOp loop,
                  mlir::PatternRewriter &rewriter) const override {
    auto loc = loop.getLoc();
    mlir::arith::IntegerOverflowFlags flags{};
    if (setNSW)
      flags = bitEnumSet(flags, mlir::arith::IntegerOverflowFlags::nsw);
    auto iofAttr = mlir::arith::IntegerOverflowFlagsAttr::get(
        rewriter.getContext(), flags);

    // Create the start and end blocks that will wrap the DoLoopOp with an
    // initalizer and an end point
    auto *initBlock = rewriter.getInsertionBlock();
    auto initPos = rewriter.getInsertionPoint();
    auto *endBlock = rewriter.splitBlock(initBlock, initPos);

    // Split the first DoLoopOp block in two parts. The part before will be the
    // conditional block since it already has the induction variable and
    // loop-carried values as arguments.
    auto *conditionalBlock = &loop.getRegion().front();
    conditionalBlock->addArgument(rewriter.getIndexType(), loc);
    auto *firstBlock =
        rewriter.splitBlock(conditionalBlock, conditionalBlock->begin());
    auto *lastBlock = &loop.getRegion().back();

    // Move the blocks from the DoLoopOp between initBlock and endBlock
    rewriter.inlineRegionBefore(loop.getRegion(), endBlock);

    // Get loop values from the DoLoopOp
    auto low = loop.getLowerBound();
    auto high = loop.getUpperBound();
    assert(low && high && "must be a Value");
    auto step = loop.getStep();

    // Initalization block
    rewriter.setInsertionPointToEnd(initBlock);
    auto diff = mlir::arith::SubIOp::create(rewriter, loc, high, low);
    auto distance = mlir::arith::AddIOp::create(rewriter, loc, diff, step);
    mlir::Value iters =
        mlir::arith::DivSIOp::create(rewriter, loc, distance, step);

    if (forceLoopToExecuteOnce) {
      auto zero = mlir::arith::ConstantIndexOp::create(rewriter, loc, 0);
      auto cond = mlir::arith::CmpIOp::create(
          rewriter, loc, arith::CmpIPredicate::sle, iters, zero);
      auto one = mlir::arith::ConstantIndexOp::create(rewriter, loc, 1);
      iters = mlir::arith::SelectOp::create(rewriter, loc, cond, one, iters);
    }

    toolchain::SmallVector<mlir::Value> loopOperands;
    loopOperands.push_back(low);
    auto operands = loop.getIterOperands();
    loopOperands.append(operands.begin(), operands.end());
    loopOperands.push_back(iters);

    mlir::cf::BranchOp::create(rewriter, loc, conditionalBlock, loopOperands);

    // Last loop block
    auto *terminator = lastBlock->getTerminator();
    rewriter.setInsertionPointToEnd(lastBlock);
    auto iv = conditionalBlock->getArgument(0);
    mlir::Value steppedIndex =
        mlir::arith::AddIOp::create(rewriter, loc, iv, step, iofAttr);
    assert(steppedIndex && "must be a Value");
    auto lastArg = conditionalBlock->getNumArguments() - 1;
    auto itersLeft = conditionalBlock->getArgument(lastArg);
    auto one = mlir::arith::ConstantIndexOp::create(rewriter, loc, 1);
    mlir::Value itersMinusOne =
        mlir::arith::SubIOp::create(rewriter, loc, itersLeft, one);

    toolchain::SmallVector<mlir::Value> loopCarried;
    loopCarried.push_back(steppedIndex);
    auto begin = loop.getFinalValue() ? std::next(terminator->operand_begin())
                                      : terminator->operand_begin();
    loopCarried.append(begin, terminator->operand_end());
    loopCarried.push_back(itersMinusOne);
    auto backEdge = mlir::cf::BranchOp::create(rewriter, loc, conditionalBlock,
                                               loopCarried);
    rewriter.eraseOp(terminator);

    // Copy loop annotations from the do loop to the loop back edge.
    if (auto ann = loop.getLoopAnnotation())
      backEdge->setAttr("loop_annotation", *ann);

    // Conditional block
    rewriter.setInsertionPointToEnd(conditionalBlock);
    auto zero = mlir::arith::ConstantIndexOp::create(rewriter, loc, 0);
    auto comparison = mlir::arith::CmpIOp::create(
        rewriter, loc, arith::CmpIPredicate::sgt, itersLeft, zero);

    mlir::cf::CondBranchOp::create(rewriter, loc, comparison, firstBlock,
                                   toolchain::ArrayRef<mlir::Value>(), endBlock,
                                   toolchain::ArrayRef<mlir::Value>());

    // The result of the loop operation is the values of the condition block
    // arguments except the induction variable on the last iteration.
    auto args = loop.getFinalValue()
                    ? conditionalBlock->getArguments()
                    : conditionalBlock->getArguments().drop_front();
    rewriter.replaceOp(loop, args.drop_back());
    return success();
  }

private:
  bool forceLoopToExecuteOnce;
  bool setNSW;
};

/// Convert `fir.if` to control-flow
class CfgIfConv : public mlir::OpRewritePattern<fir::IfOp> {
public:
  using OpRewritePattern::OpRewritePattern;

  CfgIfConv(mlir::MLIRContext *ctx, bool forceLoopToExecuteOnce, bool setNSW)
      : mlir::OpRewritePattern<fir::IfOp>(ctx) {}

  toolchain::LogicalResult
  matchAndRewrite(IfOp ifOp, mlir::PatternRewriter &rewriter) const override {
    auto loc = ifOp.getLoc();

    // Split the block containing the 'fir.if' into two parts.  The part before
    // will contain the condition, the part after will be the continuation
    // point.
    auto *condBlock = rewriter.getInsertionBlock();
    auto opPosition = rewriter.getInsertionPoint();
    auto *remainingOpsBlock = rewriter.splitBlock(condBlock, opPosition);
    mlir::Block *continueBlock;
    if (ifOp.getNumResults() == 0) {
      continueBlock = remainingOpsBlock;
    } else {
      continueBlock = rewriter.createBlock(
          remainingOpsBlock, ifOp.getResultTypes(),
          toolchain::SmallVector<mlir::Location>(ifOp.getNumResults(), loc));
      mlir::cf::BranchOp::create(rewriter, loc, remainingOpsBlock);
    }

    // Move blocks from the "then" region to the region containing 'fir.if',
    // place it before the continuation block, and branch to it.
    auto &ifOpRegion = ifOp.getThenRegion();
    auto *ifOpBlock = &ifOpRegion.front();
    auto *ifOpTerminator = ifOpRegion.back().getTerminator();
    auto ifOpTerminatorOperands = ifOpTerminator->getOperands();
    rewriter.setInsertionPointToEnd(&ifOpRegion.back());
    mlir::cf::BranchOp::create(rewriter, loc, continueBlock,
                               ifOpTerminatorOperands);
    rewriter.eraseOp(ifOpTerminator);
    rewriter.inlineRegionBefore(ifOpRegion, continueBlock);

    // Move blocks from the "else" region (if present) to the region containing
    // 'fir.if', place it before the continuation block and branch to it.  It
    // will be placed after the "then" regions.
    auto *otherwiseBlock = continueBlock;
    auto &otherwiseRegion = ifOp.getElseRegion();
    if (!otherwiseRegion.empty()) {
      otherwiseBlock = &otherwiseRegion.front();
      auto *otherwiseTerm = otherwiseRegion.back().getTerminator();
      auto otherwiseTermOperands = otherwiseTerm->getOperands();
      rewriter.setInsertionPointToEnd(&otherwiseRegion.back());
      mlir::cf::BranchOp::create(rewriter, loc, continueBlock,
                                 otherwiseTermOperands);
      rewriter.eraseOp(otherwiseTerm);
      rewriter.inlineRegionBefore(otherwiseRegion, continueBlock);
    }

    rewriter.setInsertionPointToEnd(condBlock);
    auto branchOp = mlir::cf::CondBranchOp::create(
        rewriter, loc, ifOp.getCondition(), ifOpBlock,
        toolchain::ArrayRef<mlir::Value>(), otherwiseBlock,
        toolchain::ArrayRef<mlir::Value>());
    toolchain::ArrayRef<int32_t> weights = ifOp.getWeights();
    if (!weights.empty())
      branchOp.setWeights(weights);
    rewriter.replaceOp(ifOp, continueBlock->getArguments());
    return success();
  }
};

/// Convert `fir.iter_while` to control-flow.
class CfgIterWhileConv : public mlir::OpRewritePattern<fir::IterWhileOp> {
public:
  using OpRewritePattern::OpRewritePattern;

  CfgIterWhileConv(mlir::MLIRContext *ctx, bool forceLoopToExecuteOnce,
                   bool setNSW)
      : mlir::OpRewritePattern<fir::IterWhileOp>(ctx), setNSW(setNSW) {}

  toolchain::LogicalResult
  matchAndRewrite(fir::IterWhileOp whileOp,
                  mlir::PatternRewriter &rewriter) const override {
    auto loc = whileOp.getLoc();
    mlir::arith::IntegerOverflowFlags flags{};
    if (setNSW)
      flags = bitEnumSet(flags, mlir::arith::IntegerOverflowFlags::nsw);
    auto iofAttr = mlir::arith::IntegerOverflowFlagsAttr::get(
        rewriter.getContext(), flags);

    // Start by splitting the block containing the 'fir.do_loop' into two parts.
    // The part before will get the init code, the part after will be the end
    // point.
    auto *initBlock = rewriter.getInsertionBlock();
    auto initPosition = rewriter.getInsertionPoint();
    auto *endBlock = rewriter.splitBlock(initBlock, initPosition);

    // Use the first block of the loop body as the condition block since it is
    // the block that has the induction variable and loop-carried values as
    // arguments. Split out all operations from the first block into a new
    // block. Move all body blocks from the loop body region to the region
    // containing the loop.
    auto *conditionBlock = &whileOp.getRegion().front();
    auto *firstBodyBlock =
        rewriter.splitBlock(conditionBlock, conditionBlock->begin());
    auto *lastBodyBlock = &whileOp.getRegion().back();
    rewriter.inlineRegionBefore(whileOp.getRegion(), endBlock);
    auto iv = conditionBlock->getArgument(0);
    auto iterateVar = conditionBlock->getArgument(1);

    // Append the induction variable stepping logic to the last body block and
    // branch back to the condition block. Loop-carried values are taken from
    // operands of the loop terminator.
    auto *terminator = lastBodyBlock->getTerminator();
    rewriter.setInsertionPointToEnd(lastBodyBlock);
    auto step = whileOp.getStep();
    mlir::Value stepped =
        mlir::arith::AddIOp::create(rewriter, loc, iv, step, iofAttr);
    assert(stepped && "must be a Value");

    toolchain::SmallVector<mlir::Value> loopCarried;
    loopCarried.push_back(stepped);
    auto begin = whileOp.getFinalValue()
                     ? std::next(terminator->operand_begin())
                     : terminator->operand_begin();
    loopCarried.append(begin, terminator->operand_end());
    mlir::cf::BranchOp::create(rewriter, loc, conditionBlock, loopCarried);
    rewriter.eraseOp(terminator);

    // Compute loop bounds before branching to the condition.
    rewriter.setInsertionPointToEnd(initBlock);
    auto lowerBound = whileOp.getLowerBound();
    auto upperBound = whileOp.getUpperBound();
    assert(lowerBound && upperBound && "must be a Value");

    // The initial values of loop-carried values is obtained from the operands
    // of the loop operation.
    toolchain::SmallVector<mlir::Value> destOperands;
    destOperands.push_back(lowerBound);
    auto iterOperands = whileOp.getIterOperands();
    destOperands.append(iterOperands.begin(), iterOperands.end());
    mlir::cf::BranchOp::create(rewriter, loc, conditionBlock, destOperands);

    // With the body block done, we can fill in the condition block.
    rewriter.setInsertionPointToEnd(conditionBlock);
    // The comparison depends on the sign of the step value. We fully expect
    // this expression to be folded by the optimizer or LLVM. This expression
    // is written this way so that `step == 0` always returns `false`.
    auto zero = mlir::arith::ConstantIndexOp::create(rewriter, loc, 0);
    auto compl0 = mlir::arith::CmpIOp::create(
        rewriter, loc, arith::CmpIPredicate::slt, zero, step);
    auto compl1 = mlir::arith::CmpIOp::create(
        rewriter, loc, arith::CmpIPredicate::sle, iv, upperBound);
    auto compl2 = mlir::arith::CmpIOp::create(
        rewriter, loc, arith::CmpIPredicate::slt, step, zero);
    auto compl3 = mlir::arith::CmpIOp::create(
        rewriter, loc, arith::CmpIPredicate::sle, upperBound, iv);
    auto cmp0 = mlir::arith::AndIOp::create(rewriter, loc, compl0, compl1);
    auto cmp1 = mlir::arith::AndIOp::create(rewriter, loc, compl2, compl3);
    auto cmp2 = mlir::arith::OrIOp::create(rewriter, loc, cmp0, cmp1);
    // Remember to AND in the early-exit bool.
    auto comparison =
        mlir::arith::AndIOp::create(rewriter, loc, iterateVar, cmp2);
    mlir::cf::CondBranchOp::create(rewriter, loc, comparison, firstBodyBlock,
                                   toolchain::ArrayRef<mlir::Value>(), endBlock,
                                   toolchain::ArrayRef<mlir::Value>());
    // The result of the loop operation is the values of the condition block
    // arguments except the induction variable on the last iteration.
    auto args = whileOp.getFinalValue()
                    ? conditionBlock->getArguments()
                    : conditionBlock->getArguments().drop_front();
    rewriter.replaceOp(whileOp, args);
    return success();
  }

private:
  bool setNSW;
};

/// Convert FIR structured control flow ops to CFG ops.
class CfgConversion : public fir::impl::CFGConversionBase<CfgConversion> {
public:
  using CFGConversionBase<CfgConversion>::CFGConversionBase;

  void runOnOperation() override {
    auto *context = &this->getContext();
    mlir::RewritePatternSet patterns(context);
    fir::populateCfgConversionRewrites(patterns, this->forceLoopToExecuteOnce,
                                       this->setNSW);
    mlir::ConversionTarget target(*context);
    target.addLegalDialect<mlir::affine::AffineDialect,
                           mlir::cf::ControlFlowDialect, FIROpsDialect,
                           mlir::func::FuncDialect>();

    // apply the patterns
    target.addIllegalOp<ResultOp, DoLoopOp, IfOp, IterWhileOp>();
    target.markUnknownOpDynamicallyLegal([](Operation *) { return true; });
    if (mlir::failed(mlir::applyPartialConversion(this->getOperation(), target,
                                                  std::move(patterns)))) {
      mlir::emitError(mlir::UnknownLoc::get(context),
                      "error in converting to CFG\n");
      this->signalPassFailure();
    }
  }
};

} // namespace

/// Expose conversion rewriters to other passes
void fir::populateCfgConversionRewrites(mlir::RewritePatternSet &patterns,
                                        bool forceLoopToExecuteOnce,
                                        bool setNSW) {
  patterns.insert<CfgLoopConv, CfgIfConv, CfgIterWhileConv>(
      patterns.getContext(), forceLoopToExecuteOnce, setNSW);
}
