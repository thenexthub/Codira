//===- MorphOps.cpp - conversion between named,category and generic ops ---===//
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
// This file implements conversions between linalg ops:
//    named <--> category (elementwise, contraction, ..) <--> generic.
//===----------------------------------------------------------------------===//

#include "mlir/Dialect/Complex/IR/Complex.h"
#include "mlir/Dialect/Linalg/IR/Linalg.h"
#include "mlir/Dialect/Linalg/IR/LinalgInterfaces.h"
#include "mlir/Dialect/Linalg/Passes.h"
#include "mlir/Dialect/Linalg/Transforms/Transforms.h"
#include "mlir/Dialect/Math/IR/Math.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"

namespace mlir {
#define GEN_PASS_DEF_LINALGMORPHOPSPASS
#include "mlir/Dialect/Linalg/Passes.h.inc"
} // namespace mlir

#define DEBUG_TYPE "linalg-morphism"

using namespace mlir;
using namespace mlir::linalg;

namespace {
struct LinalgMorphOpsPass
    : public impl::LinalgMorphOpsPassBase<LinalgMorphOpsPass> {

  using impl::LinalgMorphOpsPassBase<
      LinalgMorphOpsPass>::LinalgMorphOpsPassBase;

  void runOnOperation() override;
};

void LinalgMorphOpsPass::runOnOperation() {

  RewritePatternSet patterns(&getContext());

  // Lowering paths (named -> category -> generic)
  if (namedToCategory) {
    populateLinalgNamedToElementwisePatterns(patterns);
  }
  if (namedToGeneric || categoryToGeneric) {
    populateLinalgNamedOpsGeneralizationPatterns(patterns);
  }

  // Lifting paths (named <- category <- generic)
  if (genericToNamed) {
    populateLinalgGenericOpsSpecializationPatterns(patterns);
  }

  if (failed(applyPatternsGreedily(getOperation(), std::move(patterns))))
    signalPassFailure();
}
} // namespace
