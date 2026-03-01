//===- FormExpressions.cpp - Form C-style expressions --------*- C++ -*-===//
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
// This file implements a pass that forms EmitC operations modeling C operators
// into C-style expressions using the emitc.expression op.
//
//===----------------------------------------------------------------------===//

#include "mlir/Dialect/EmitC/IR/EmitC.h"
#include "mlir/Dialect/EmitC/Transforms/Passes.h"
#include "mlir/Dialect/EmitC/Transforms/Transforms.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"

namespace mlir {
namespace emitc {
#define GEN_PASS_DEF_FORMEXPRESSIONSPASS
#include "mlir/Dialect/EmitC/Transforms/Passes.h.inc"
} // namespace emitc
} // namespace mlir

using namespace mlir;
using namespace emitc;

namespace {
struct FormExpressionsPass
    : public emitc::impl::FormExpressionsPassBase<FormExpressionsPass> {
  void runOnOperation() override {
    Operation *rootOp = getOperation();
    MLIRContext *context = rootOp->getContext();

    // Wrap each C operator op with an expression op.
    OpBuilder builder(context);
    auto matchFun = [&](Operation *op) {
      if (isa<emitc::CExpressionInterface>(*op) &&
          !op->getParentOfType<emitc::ExpressionOp>() &&
          op->getNumResults() == 1)
        createExpression(op, builder);
    };
    rootOp->walk(matchFun);

    // Fold expressions where possible.
    RewritePatternSet patterns(context);
    populateExpressionPatterns(patterns);

    if (failed(applyPatternsGreedily(rootOp, std::move(patterns))))
      return signalPassFailure();
  }

  void getDependentDialects(DialectRegistry &registry) const override {
    registry.insert<emitc::EmitCDialect>();
  }
};
} // namespace
