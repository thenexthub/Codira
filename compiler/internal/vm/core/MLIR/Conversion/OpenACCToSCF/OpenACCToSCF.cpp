//===- OpenACCToSCF.cpp - OpenACC condition to SCF if conversion ----------===//
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

#include "mlir/Conversion/OpenACCToSCF/ConvertOpenACCToSCF.h"

#include "mlir/Dialect/OpenACC/OpenACC.h"
#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/IR/Matchers.h"
#include "mlir/Transforms/DialectConversion.h"

namespace mlir {
#define GEN_PASS_DEF_CONVERTOPENACCTOSCFPASS
#include "mlir/Conversion/Passes.h.inc"
} // namespace mlir

using namespace mlir;

//===----------------------------------------------------------------------===//
// Conversion patterns
//===----------------------------------------------------------------------===//

namespace {
/// Pattern to transform the `getIfCond` on operation without region into a
/// scf.if and move the operation into the `then` region.
template <typename OpTy>
class ExpandIfCondition : public OpRewritePattern<OpTy> {
  using OpRewritePattern<OpTy>::OpRewritePattern;

  LogicalResult matchAndRewrite(OpTy op,
                                PatternRewriter &rewriter) const override {
    // Early exit if there is no condition.
    if (!op.getIfCond())
      return failure();

    IntegerAttr constAttr;
    if (!matchPattern(op.getIfCond(), m_Constant(&constAttr))) {
      auto ifOp = scf::IfOp::create(rewriter, op.getLoc(), TypeRange(),
                                    op.getIfCond(), false);
      rewriter.modifyOpInPlace(op, [&]() { op.getIfCondMutable().erase(0); });
      auto thenBodyBuilder = ifOp.getThenBodyBuilder(rewriter.getListener());
      thenBodyBuilder.clone(*op.getOperation());
      rewriter.eraseOp(op);
    } else {
      if (constAttr.getInt())
        rewriter.modifyOpInPlace(op, [&]() { op.getIfCondMutable().erase(0); });
      else
        rewriter.eraseOp(op);
    }
    return success();
  }
};
} // namespace

void mlir::populateOpenACCToSCFConversionPatterns(RewritePatternSet &patterns) {
  patterns.add<ExpandIfCondition<acc::EnterDataOp>>(patterns.getContext());
  patterns.add<ExpandIfCondition<acc::ExitDataOp>>(patterns.getContext());
  patterns.add<ExpandIfCondition<acc::UpdateOp>>(patterns.getContext());
}

namespace {
struct ConvertOpenACCToSCFPass
    : public impl::ConvertOpenACCToSCFPassBase<ConvertOpenACCToSCFPass> {
  void runOnOperation() override;
};
} // namespace

void ConvertOpenACCToSCFPass::runOnOperation() {
  auto op = getOperation();
  auto *context = op.getContext();

  RewritePatternSet patterns(context);
  ConversionTarget target(*context);
  populateOpenACCToSCFConversionPatterns(patterns);

  target.addLegalDialect<scf::SCFDialect>();
  target.addLegalDialect<acc::OpenACCDialect>();

  target.addDynamicallyLegalOp<acc::EnterDataOp>(
      [](acc::EnterDataOp op) { return !op.getIfCond(); });

  target.addDynamicallyLegalOp<acc::ExitDataOp>(
      [](acc::ExitDataOp op) { return !op.getIfCond(); });

  target.addDynamicallyLegalOp<acc::UpdateOp>(
      [](acc::UpdateOp op) { return !op.getIfCond(); });

  if (failed(applyPartialConversion(op, target, std::move(patterns))))
    signalPassFailure();
}
