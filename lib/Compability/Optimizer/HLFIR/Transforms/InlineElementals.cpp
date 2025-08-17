//===- InlineElementals.cpp - Inline chained hlfir.elemental ops ----------===//
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
// Chained elemental operations like a + b + c can inline the first elemental
// at the hlfir.apply in the body of the second one (as described in
// docs/HighLevelFIR.md). This has to be done in a pass rather than in lowering
// so that it happens after the HLFIR intrinsic simplification pass.
//===----------------------------------------------------------------------===//

#include "language/Compability/Optimizer/Builder/FIRBuilder.h"
#include "language/Compability/Optimizer/Builder/HLFIRTools.h"
#include "language/Compability/Optimizer/Dialect/Support/FIRContext.h"
#include "language/Compability/Optimizer/HLFIR/HLFIROps.h"
#include "language/Compability/Optimizer/HLFIR/Passes.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/IR/IRMapping.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Support/LLVM.h"
#include "mlir/Transforms/DialectConversion.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"
#include "toolchain/ADT/TypeSwitch.h"
#include <iterator>

namespace hlfir {
#define GEN_PASS_DEF_INLINEELEMENTALS
#include "language/Compability/Optimizer/HLFIR/Passes.h.inc"
} // namespace hlfir

/// If the elemental has only two uses and those two are an apply operation and
/// a destroy operation, return those two, otherwise return {}
static std::optional<std::pair<hlfir::ApplyOp, hlfir::DestroyOp>>
getTwoUses(hlfir::ElementalOp elemental) {
  mlir::Operation::user_range users = elemental->getUsers();
  // don't inline anything with more than one use (plus hfir.destroy)
  if (std::distance(users.begin(), users.end()) != 2) {
    return std::nullopt;
  }

  // If the ElementalOp must produce a temporary (e.g. for
  // finalization purposes), then we cannot inline it.
  if (hlfir::elementalOpMustProduceTemp(elemental))
    return std::nullopt;

  hlfir::ApplyOp apply;
  hlfir::DestroyOp destroy;
  for (mlir::Operation *user : users)
    mlir::TypeSwitch<mlir::Operation *, void>(user)
        .Case([&](hlfir::ApplyOp op) { apply = op; })
        .Case([&](hlfir::DestroyOp op) { destroy = op; });

  if (!apply || !destroy)
    return std::nullopt;

  // we can't inline if the return type of the yield doesn't match the return
  // type of the apply
  auto yield = mlir::dyn_cast_or_null<hlfir::YieldElementOp>(
      elemental.getRegion().back().back());
  assert(yield && "hlfir.elemental should always end with a yield");
  if (apply.getResult().getType() != yield.getElementValue().getType())
    return std::nullopt;

  return std::pair{apply, destroy};
}

namespace {
class InlineElementalConversion
    : public mlir::OpRewritePattern<hlfir::ElementalOp> {
public:
  using mlir::OpRewritePattern<hlfir::ElementalOp>::OpRewritePattern;

  toolchain::LogicalResult
  matchAndRewrite(hlfir::ElementalOp elemental,
                  mlir::PatternRewriter &rewriter) const override {
    std::optional<std::pair<hlfir::ApplyOp, hlfir::DestroyOp>> maybeTuple =
        getTwoUses(elemental);
    if (!maybeTuple)
      return rewriter.notifyMatchFailure(
          elemental, "hlfir.elemental does not have two uses");

    if (elemental.isOrdered()) {
      // We can only inline the ordered elemental into a loop-like
      // construct that processes the indices in-order and does not
      // have the side effects itself. Adhere to conservative behavior
      // for the time being.
      return rewriter.notifyMatchFailure(elemental,
                                         "hlfir.elemental is ordered");
    }
    auto [apply, destroy] = *maybeTuple;

    assert(elemental.getRegion().hasOneBlock() &&
           "expect elemental region to have one block");

    fir::FirOpBuilder builder{rewriter, elemental.getOperation()};
    builder.setInsertionPointAfter(apply);
    hlfir::YieldElementOp yield = hlfir::inlineElementalOp(
        elemental.getLoc(), builder, elemental, apply.getIndices());

    // remove the old elemental and all of the bookkeeping
    rewriter.replaceAllUsesWith(apply.getResult(), yield.getElementValue());
    rewriter.eraseOp(yield);
    rewriter.eraseOp(apply);
    rewriter.eraseOp(destroy);
    rewriter.eraseOp(elemental);

    return mlir::success();
  }
};

class InlineElementalsPass
    : public hlfir::impl::InlineElementalsBase<InlineElementalsPass> {
public:
  void runOnOperation() override {
    mlir::MLIRContext *context = &getContext();

    mlir::GreedyRewriteConfig config;
    // Prevent the pattern driver from merging blocks.
    config.setRegionSimplificationLevel(
        mlir::GreedySimplifyRegionLevel::Disabled);

    mlir::RewritePatternSet patterns(context);
    patterns.insert<InlineElementalConversion>(context);

    if (mlir::failed(mlir::applyPatternsGreedily(
            getOperation(), std::move(patterns), config))) {
      mlir::emitError(getOperation()->getLoc(),
                      "failure in HLFIR elemental inlining");
      signalPassFailure();
    }
  }
};
} // namespace
