//===- TosaLayerwiseConstantFoldPass.cpp ----------------------------------===//
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
// This file implements constant folding transformations on TOSA operations
//
//===----------------------------------------------------------------------===//

#include "mlir/Dialect/Tosa/Transforms/Passes.h"

#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"

namespace mlir {
namespace tosa {
#define GEN_PASS_DEF_TOSALAYERWISECONSTANTFOLDPASS
#include "mlir/Dialect/Tosa/Transforms/Passes.h.inc"
} // namespace tosa
} // namespace mlir

using namespace mlir;
using namespace mlir::tosa;

namespace {

template <typename... Args>
void addOpsCanonicalizations(MLIRContext *ctx, RewritePatternSet &patterns) {
  (Args::getCanonicalizationPatterns(patterns, ctx), ...);
}

void populateTosaOpsCanonicalizationPatterns(MLIRContext *ctx,
                                             RewritePatternSet &patterns) {
  addOpsCanonicalizations<
#define GET_OP_LIST
#include "mlir/Dialect/Tosa/IR/TosaOps.cpp.inc"
      >(ctx, patterns);
}

struct TosaLayerwiseConstantFoldPass
    : public tosa::impl::TosaLayerwiseConstantFoldPassBase<
          TosaLayerwiseConstantFoldPass> {
  using Base::Base;

  void runOnOperation() override {
    auto *ctx = &getContext();
    RewritePatternSet patterns(ctx);
    auto func = getOperation();

    mlir::tosa::populateTosaFoldConstantReciprocalPatterns(ctx, patterns);
    mlir::tosa::populateTosaFoldConstantTransposePatterns(ctx, patterns);
    mlir::tosa::populateTosaConstantReduction(ctx, patterns,
                                              aggressiveReduceConstant);
    populateTosaOpsCanonicalizationPatterns(ctx, patterns);

    if (applyPatternsGreedily(func, std::move(patterns)).failed())
      signalPassFailure();
  }
};

} // namespace
