//===- ParallelLoopTiling.cpp - Tiles scf.parallel ------------------------===//
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
// This file implements loop tiling on parallel loops.
//
//===----------------------------------------------------------------------===//

#include "mlir/Dialect/SCF/Transforms/Passes.h"

#include "mlir/Dialect/Affine/IR/AffineOps.h"
#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/Dialect/SCF/Transforms/Transforms.h"
#include "mlir/Dialect/SCF/Utils/Utils.h"

namespace mlir {
#define GEN_PASS_DEF_SCFPARALLELLOOPTILING
#include "mlir/Dialect/SCF/Transforms/Passes.h.inc"
} // namespace mlir

using namespace mlir;
using namespace mlir::scf;

/// Tile a parallel loop of the form
///   scf.parallel (%i0, %i1) = (%arg0, %arg1) to (%arg2, %arg3)
///                                            step (%arg4, %arg5)
///
/// into
///   scf.parallel (%i0, %i1) = (%arg0, %arg1) to (%arg2, %arg3)
///                                            step (%arg4*tileSize[0],
///                                                  %arg5*tileSize[1])
///     scf.parallel (%j0, %j1) = (0, 0) to (min(%arg4*tileSize[0], %arg2-%i0)
///                                          min(%arg5*tileSize[1], %arg3-%i1))
///                                      step (%arg4, %arg5)
///
/// or, when no-min-max-bounds is true, into
///   scf.parallel (%i0, %i1) = (%arg0, %arg1) to (%arg2, %arg3)
///                                            step (%arg4*tileSize[0],
///                                                  %arg5*tileSize[1])
///     scf.parallel (%j0, %j1) = (0, 0) to (%arg4*tileSize[0],
///                                          %arg5*tileSize[1])
///                                      step (%arg4, %arg5)
///        %inbound = (%j0 * %arg4 + %i0 < %arg2) &&
///                   (%j1 * %arg5 + %i1 < %arg3)
///        scf.if (%inbound)
///          ....
///
/// where the uses of %i0 and %i1 in the loop body are replaced by
/// %i0 + j0 and %i1 + %j1.
///
/// The old loop is replaced with the new one.
std::pair<ParallelOp, ParallelOp>
mlir::scf::tileParallelLoop(ParallelOp op, ArrayRef<int64_t> tileSizes,
                            bool noMinMaxBounds) {
  OpBuilder b(op);
  auto zero = arith::ConstantIndexOp::create(b, op.getLoc(), 0);
  SmallVector<Value, 2> tileSizeConstants;
  tileSizeConstants.reserve(op.getUpperBound().size());
  for (size_t i = 0, end = op.getUpperBound().size(); i != end; ++i) {
    if (i < tileSizes.size())
      tileSizeConstants.push_back(
          arith::ConstantIndexOp::create(b, op.getLoc(), tileSizes[i]));
    else
      // Just pick 1 for the remaining dimensions.
      tileSizeConstants.push_back(
          arith::ConstantIndexOp::create(b, op.getLoc(), 1));
  }

  // Create the outer loop with adjusted steps.
  SmallVector<Value, 2> newSteps;
  newSteps.reserve(op.getStep().size());
  for (auto step : toolchain::zip(op.getStep(), tileSizeConstants)) {
    newSteps.push_back(arith::MulIOp::create(b, op.getLoc(), std::get<0>(step),
                                             std::get<1>(step)));
  }
  auto outerLoop = ParallelOp::create(b, op.getLoc(), op.getLowerBound(),
                                      op.getUpperBound(), newSteps);
  b.setInsertionPointToStart(outerLoop.getBody());

  // Compute min(size, dim - offset) to avoid out-of-bounds accesses.
  auto minMap = AffineMap::get(
      /*dimCount=*/3, /*symbolCount=*/0,
      {getAffineDimExpr(/*position=*/0, b.getContext()),
       getAffineDimExpr(/*position=*/1, b.getContext()) -
           getAffineDimExpr(/*position=*/2, b.getContext())},
      b.getContext());

  // Create the inner loop with adjusted bounds.
  SmallVector<Value, 2> newBounds;
  newBounds.reserve(op.getUpperBound().size());
  bool needInboundCheck = false;
  for (auto [lowerBound, upperBound, newStep, iv, step, tileSizeConstant] :
       toolchain::zip(outerLoop.getLowerBound(), outerLoop.getUpperBound(),
                 outerLoop.getStep(), outerLoop.getInductionVars(),
                 op.getStep(), tileSizeConstants)) {
    // Collect the statically known loop bounds
    auto lowerBoundConstant =
        lowerBound.getDefiningOp<arith::ConstantIndexOp>();
    auto upperBoundConstant =
        upperBound.getDefiningOp<arith::ConstantIndexOp>();
    auto stepConstant = step.getDefiningOp<arith::ConstantIndexOp>();
    auto tileSize =
        cast<arith::ConstantIndexOp>(tileSizeConstant.getDefiningOp()).value();
    // If the loop bounds and the loop step are constant and if the number of
    // loop iterations is an integer multiple of the tile size, we use a static
    // bound for the inner loop.
    if (lowerBoundConstant && upperBoundConstant && stepConstant) {
      auto numIterations = toolchain::divideCeil(upperBoundConstant.value() -
                                                lowerBoundConstant.value(),
                                            stepConstant.value());
      if (numIterations % tileSize == 0) {
        newBounds.push_back(newStep);
        continue;
      }
    }

    // For InboundCheck mode, just use the variable outer step
    if (noMinMaxBounds) {
      newBounds.push_back(newStep);
      needInboundCheck = true;
      continue;
    }

    // Otherwise, we dynamically compute the bound for
    // each iteration of the outer loop.
    newBounds.push_back(
        affine::AffineMinOp::create(b, op.getLoc(), b.getIndexType(), minMap,
                                    ValueRange{newStep, upperBound, iv}));
  }
  auto innerLoop = ParallelOp::create(
      b, op.getLoc(), SmallVector<Value, 2>(newBounds.size(), zero), newBounds,
      op.getStep());

  if (noMinMaxBounds && needInboundCheck) {
    b.setInsertionPointToStart(innerLoop.getBody());
    // Insert in-bound check
    Value inbound =
        arith::ConstantIntOp::create(b, op.getLoc(), b.getIntegerType(1), 1);
    for (auto [outerUpperBound, outerIV, innerIV, innerStep] :
         toolchain::zip(outerLoop.getUpperBound(), outerLoop.getInductionVars(),
                   innerLoop.getInductionVars(), innerLoop.getStep())) {
      // %in_bound = %in_bound &&
      //             (%inner_iv * %inner_step + %outer_iv < %outer_upper_bound)
      Value index = arith::AddIOp::create(
          b, op.getLoc(),
          arith::MulIOp::create(b, op.getLoc(), innerIV, innerStep), outerIV);
      Value dimInbound = arith::CmpIOp::create(
          b, op.getLoc(), arith::CmpIPredicate::ult, index, outerUpperBound);
      inbound = arith::AndIOp::create(b, op.getLoc(), inbound, dimInbound);
    }
    auto ifInbound = IfOp::create(b, op.getLoc(),
                                  /*resultTypes*/ ArrayRef<Type>{}, inbound,
                                  /*hasElseRegion*/ false);
    ifInbound.getThenRegion().takeBody(op.getRegion());
    Block &thenBlock = ifInbound.getThenRegion().front();
    // Replace the scf.reduce terminator with an scf.yield terminator.
    Operation *reduceOp = thenBlock.getTerminator();
    b.setInsertionPointToEnd(&thenBlock);
    scf::YieldOp::create(b, reduceOp->getLoc());
    reduceOp->erase();
    b.setInsertionPointToStart(innerLoop.getBody());
    for (const auto &ivs : toolchain::enumerate(toolchain::zip(
             innerLoop.getInductionVars(), outerLoop.getInductionVars()))) {
      auto newIndex = arith::AddIOp::create(
          b, op.getLoc(), std::get<0>(ivs.value()), std::get<1>(ivs.value()));
      thenBlock.getArgument(ivs.index())
          .replaceAllUsesExcept(newIndex, newIndex);
    }
    thenBlock.eraseArguments(0, thenBlock.getNumArguments());
  } else {
    innerLoop.getRegion().takeBody(op.getRegion());
    b.setInsertionPointToStart(innerLoop.getBody());
    for (auto ivs : toolchain::zip(innerLoop.getInductionVars(),
                              outerLoop.getInductionVars())) {
      Value innerIndex = std::get<0>(ivs);
      auto newIndex = arith::AddIOp::create(b, op.getLoc(), std::get<0>(ivs),
                                            std::get<1>(ivs));
      innerIndex.replaceAllUsesExcept(newIndex, newIndex);
    }
  }

  op.erase();
  return std::make_pair(outerLoop, innerLoop);
}

namespace {
struct ParallelLoopTiling
    : public impl::SCFParallelLoopTilingBase<ParallelLoopTiling> {
  ParallelLoopTiling() = default;
  explicit ParallelLoopTiling(ArrayRef<int64_t> tileSizes,
                              bool noMinMaxBounds = false) {
    this->tileSizes = tileSizes;
    this->noMinMaxBounds = noMinMaxBounds;
  }

  void runOnOperation() override {
    for (auto tileSize : tileSizes)
      if (tileSize == 0) {
        mlir::emitError(mlir::UnknownLoc::get(&Pass::getContext()),
                        "tile size cannot be 0");
        return signalPassFailure();
      }
    auto *parentOp = getOperation();
    SmallVector<ParallelOp, 2> innermostPloops;
    getInnermostParallelLoops(parentOp, innermostPloops);
    for (ParallelOp ploop : innermostPloops) {
      // FIXME: Add reduction support.
      if (ploop.getNumReductions() == 0)
        tileParallelLoop(ploop, tileSizes, noMinMaxBounds);
    }
  }
};
} // namespace

std::unique_ptr<Pass>
mlir::createParallelLoopTilingPass(ArrayRef<int64_t> tileSizes,
                                   bool noMinMaxBounds) {
  return std::make_unique<ParallelLoopTiling>(tileSizes, noMinMaxBounds);
}
