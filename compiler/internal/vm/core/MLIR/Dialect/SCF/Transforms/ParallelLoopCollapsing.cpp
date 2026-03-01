//===- ParallelLoopCollapsing.cpp - Pass collapsing parallel loop indices -===//
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

#include "mlir/Dialect/SCF/Transforms/Passes.h"

#include "mlir/Dialect/Affine/IR/AffineOps.h"
#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/Dialect/SCF/Utils/Utils.h"
#include "vm/core/ADT/SmallSet.h"
#include "vm/core/Support/Debug.h"

namespace mlir {
#define GEN_PASS_DEF_TESTSCFPARALLELLOOPCOLLAPSING
#include "mlir/Dialect/SCF/Transforms/Passes.h.inc"
} // namespace mlir

#define DEBUG_TYPE "parallel-loop-collapsing"

using namespace mlir;

namespace {
struct TestSCFParallelLoopCollapsing
    : public impl::TestSCFParallelLoopCollapsingBase<
          TestSCFParallelLoopCollapsing> {

  void runOnOperation() override {
    Operation *module = getOperation();

    // The common case for GPU dialect will be simplifying the ParallelOp to 3
    // arguments, so we do that here to simplify things.
    toolchain::SmallVector<std::vector<unsigned>, 3> combinedLoops;

    // Gather the input args into the format required by
    // `collapseParallelLoops`.
    if (!clCollapsedIndices0.empty())
      combinedLoops.push_back(clCollapsedIndices0);
    if (!clCollapsedIndices1.empty()) {
      if (clCollapsedIndices0.empty()) {
        toolchain::errs()
            << "collapsed-indices-1 specified but not collapsed-indices-0";
        signalPassFailure();
        return;
      }
      combinedLoops.push_back(clCollapsedIndices1);
    }
    if (!clCollapsedIndices2.empty()) {
      if (clCollapsedIndices1.empty()) {
        toolchain::errs()
            << "collapsed-indices-2 specified but not collapsed-indices-1";
        signalPassFailure();
        return;
      }
      combinedLoops.push_back(clCollapsedIndices2);
    }

    if (combinedLoops.empty()) {
      toolchain::errs() << "No collapsed-indices were specified. This pass is only "
                      "for testing and does not automatically collapse all "
                      "parallel loops or similar.";
      signalPassFailure();
      return;
    }

    // Confirm that the specified loops are [0,N) by testing that N values exist
    // with the maximum value being N-1.
    toolchain::SmallSet<unsigned, 8> flattenedCombinedLoops;
    unsigned maxCollapsedIndex = 0;
    for (auto &loops : combinedLoops) {
      for (auto &loop : loops) {
        flattenedCombinedLoops.insert(loop);
        maxCollapsedIndex = std::max(maxCollapsedIndex, loop);
      }
    }

    if (maxCollapsedIndex != flattenedCombinedLoops.size() - 1 ||
        !flattenedCombinedLoops.contains(maxCollapsedIndex)) {
      toolchain::errs()
          << "collapsed-indices arguments must include all values [0,N).";
      signalPassFailure();
      return;
    }

    // Only apply the transformation on parallel loops where the specified
    // transformation is valid, but do NOT early abort in the case of invalid
    // loops.
    IRRewriter rewriter(&getContext());
    module->walk([&](scf::ParallelOp op) {
      if (flattenedCombinedLoops.size() != op.getNumLoops()) {
        op.emitOpError("has ")
            << op.getNumLoops()
            << " iter args while this limited functionality testing pass was "
               "configured only for loops with exactly "
            << flattenedCombinedLoops.size() << " iter args.";
        return;
      }
      collapseParallelLoops(rewriter, op, combinedLoops);
    });
  }
};
} // namespace

std::unique_ptr<Pass> mlir::createTestSCFParallelLoopCollapsingPass() {
  return std::make_unique<TestSCFParallelLoopCollapsing>();
}
