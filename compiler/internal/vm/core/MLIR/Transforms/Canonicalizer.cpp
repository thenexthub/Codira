//===- Canonicalizer.cpp - Canonicalize MLIR operations -------------------===//
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
// This transformation pass converts operations into their canonical forms by
// folding constants, applying operation identity transformations etc.
//
//===----------------------------------------------------------------------===//

#include "mlir/Transforms/Passes.h"

#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"

namespace mlir {
#define GEN_PASS_DEF_CANONICALIZER
#include "mlir/Transforms/Passes.h.inc"
} // namespace mlir

using namespace mlir;

namespace {
/// Canonicalize operations in nested regions.
struct Canonicalizer : public impl::CanonicalizerBase<Canonicalizer> {
  Canonicalizer() = default;
  Canonicalizer(const GreedyRewriteConfig &config,
                ArrayRef<std::string> disabledPatterns,
                ArrayRef<std::string> enabledPatterns)
      : config(config) {
    this->topDownProcessingEnabled = config.getUseTopDownTraversal();
    this->regionSimplifyLevel = config.getRegionSimplificationLevel();
    this->maxIterations = config.getMaxIterations();
    this->maxNumRewrites = config.getMaxNumRewrites();
    this->disabledPatterns = disabledPatterns;
    this->enabledPatterns = enabledPatterns;
  }

  /// Initialize the canonicalizer by building the set of patterns used during
  /// execution.
  LogicalResult initialize(MLIRContext *context) override {
    // Set the config from possible pass options set in the meantime.
    config.setUseTopDownTraversal(topDownProcessingEnabled);
    config.setRegionSimplificationLevel(regionSimplifyLevel);
    config.setMaxIterations(maxIterations);
    config.setMaxNumRewrites(maxNumRewrites);

    RewritePatternSet owningPatterns(context);
    for (auto *dialect : context->getLoadedDialects())
      dialect->getCanonicalizationPatterns(owningPatterns);
    for (RegisteredOperationName op : context->getRegisteredOperations())
      op.getCanonicalizationPatterns(owningPatterns, context);

    patterns = std::make_shared<FrozenRewritePatternSet>(
        std::move(owningPatterns), disabledPatterns, enabledPatterns);
    return success();
  }
  void runOnOperation() override {
    LogicalResult converged =
        applyPatternsGreedily(getOperation(), *patterns, config);
    // Canonicalization is best-effort. Non-convergence is not a pass failure.
    if (testConvergence && failed(converged))
      signalPassFailure();
  }
  GreedyRewriteConfig config;
  std::shared_ptr<const FrozenRewritePatternSet> patterns;
};
} // namespace

/// Create a Canonicalizer pass.
std::unique_ptr<Pass> mlir::createCanonicalizerPass() {
  return std::make_unique<Canonicalizer>();
}

/// Creates an instance of the Canonicalizer pass with the specified config.
std::unique_ptr<Pass>
mlir::createCanonicalizerPass(const GreedyRewriteConfig &config,
                              ArrayRef<std::string> disabledPatterns,
                              ArrayRef<std::string> enabledPatterns) {
  return std::make_unique<Canonicalizer>(config, disabledPatterns,
                                         enabledPatterns);
}
