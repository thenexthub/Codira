//===- SimplifyRegionLite.cpp -- region simplification lite ---------------===//
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

#include "language/Compability/Optimizer/Dialect/FIROps.h"
#include "language/Compability/Optimizer/Transforms/Passes.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/DialectConversion.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"
#include "mlir/Transforms/RegionUtils.h"

namespace fir {
#define GEN_PASS_DEF_SIMPLIFYREGIONLITE
#include "language/Compability/Optimizer/Transforms/Passes.h.inc"
} // namespace fir

namespace {

class SimplifyRegionLitePass
    : public fir::impl::SimplifyRegionLiteBase<SimplifyRegionLitePass> {
public:
  void runOnOperation() override;
};

class DummyRewriter : public mlir::PatternRewriter {
public:
  DummyRewriter(mlir::MLIRContext *ctx) : mlir::PatternRewriter(ctx) {}
};

} // namespace

void SimplifyRegionLitePass::runOnOperation() {
  auto op = getOperation();
  auto regions = op->getRegions();
  mlir::RewritePatternSet patterns(op.getContext());
  DummyRewriter rewriter(op.getContext());
  if (regions.empty())
    return;

  (void)mlir::eraseUnreachableBlocks(rewriter, regions);
  (void)mlir::runRegionDCE(rewriter, regions);
}
