//===----------------------------------------------------------------------===//
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
//
// This file implements pass that canonicalizes CIR operations, eliminating
// redundant branches, empty scopes, and other unnecessary operations.
//
//===----------------------------------------------------------------------===//

#include "PassDetail.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/IR/Block.h"
#include "mlir/IR/Operation.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/IR/Region.h"
#include "mlir/Support/LogicalResult.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"
#include "language/Core/CIR/Dialect/IR/CIRDialect.h"
#include "language/Core/CIR/Dialect/Passes.h"
#include "language/Core/CIR/MissingFeatures.h"

using namespace mlir;
using namespace cir;

namespace {

/// Removes branches between two blocks if it is the only branch.
///
/// From:
///   ^bb0:
///     cir.br ^bb1
///   ^bb1:  // pred: ^bb0
///     cir.return
///
/// To:
///   ^bb0:
///     cir.return
struct RemoveRedundantBranches : public OpRewritePattern<BrOp> {
  using OpRewritePattern<BrOp>::OpRewritePattern;

  LogicalResult matchAndRewrite(BrOp op,
                                PatternRewriter &rewriter) const final {
    Block *block = op.getOperation()->getBlock();
    Block *dest = op.getDest();

    if (isa<cir::LabelOp>(dest->front()))
      return failure();
    // Single edge between blocks: merge it.
    if (block->getNumSuccessors() == 1 &&
        dest->getSinglePredecessor() == block) {
      rewriter.eraseOp(op);
      rewriter.mergeBlocks(dest, block);
      return success();
    }

    return failure();
  }
};

struct RemoveEmptyScope : public OpRewritePattern<ScopeOp> {
  using OpRewritePattern<ScopeOp>::OpRewritePattern;

  LogicalResult matchAndRewrite(ScopeOp op,
                                PatternRewriter &rewriter) const final {
    // TODO: Remove this logic once CIR uses MLIR infrastructure to remove
    // trivially dead operations
    if (op.isEmpty()) {
      rewriter.eraseOp(op);
      return success();
    }

    Region &region = op.getScopeRegion();
    if (region.getBlocks().front().getOperations().size() == 1 &&
        isa<YieldOp>(region.getBlocks().front().front())) {
      rewriter.eraseOp(op);
      return success();
    }

    return failure();
  }
};

struct RemoveEmptySwitch : public OpRewritePattern<SwitchOp> {
  using OpRewritePattern<SwitchOp>::OpRewritePattern;

  LogicalResult matchAndRewrite(SwitchOp op,
                                PatternRewriter &rewriter) const final {
    if (!(op.getBody().empty() || isa<YieldOp>(op.getBody().front().front())))
      return failure();

    rewriter.eraseOp(op);
    return success();
  }
};

//===----------------------------------------------------------------------===//
// CIRCanonicalizePass
//===----------------------------------------------------------------------===//

struct CIRCanonicalizePass : public CIRCanonicalizeBase<CIRCanonicalizePass> {
  using CIRCanonicalizeBase::CIRCanonicalizeBase;

  // The same operation rewriting done here could have been performed
  // by CanonicalizerPass (adding hasCanonicalizer for target Ops and
  // implementing the same from above in CIRDialects.cpp). However, it's
  // currently too aggressive for static analysis purposes, since it might
  // remove things where a diagnostic can be generated.
  //
  // FIXME: perhaps we can add one more mode to GreedyRewriteConfig to
  // disable this behavior.
  void runOnOperation() override;
};

void populateCIRCanonicalizePatterns(RewritePatternSet &patterns) {
  // clang-format off
  patterns.add<
    RemoveRedundantBranches,
    RemoveEmptyScope
  >(patterns.getContext());
  // clang-format on
}

void CIRCanonicalizePass::runOnOperation() {
  // Collect rewrite patterns.
  RewritePatternSet patterns(&getContext());
  populateCIRCanonicalizePatterns(patterns);

  // Collect operations to apply patterns.
  toolchain::SmallVector<Operation *, 16> ops;
  getOperation()->walk([&](Operation *op) {
    assert(!cir::MissingFeatures::switchOp());
    assert(!cir::MissingFeatures::tryOp());
    assert(!cir::MissingFeatures::complexRealOp());
    assert(!cir::MissingFeatures::complexImagOp());
    assert(!cir::MissingFeatures::callOp());

    // Many operations are here to perform a manual `fold` in
    // applyOpPatternsGreedily.
    if (isa<BrOp, BrCondOp, CastOp, ScopeOp, SwitchOp, SelectOp, UnaryOp,
            ComplexCreateOp, ComplexImagOp, ComplexRealOp, VecCmpOp,
            VecCreateOp, VecExtractOp, VecShuffleOp, VecShuffleDynamicOp,
            VecTernaryOp, BitClrsbOp, BitClzOp, BitCtzOp, BitFfsOp, BitParityOp,
            BitPopcountOp, BitReverseOp, ByteSwapOp, RotateOp>(op))
      ops.push_back(op);
  });

  // Apply patterns.
  if (applyOpPatternsGreedily(ops, std::move(patterns)).failed())
    signalPassFailure();
}

} // namespace

std::unique_ptr<Pass> mlir::createCIRCanonicalizePass() {
  return std::make_unique<CIRCanonicalizePass>();
}
