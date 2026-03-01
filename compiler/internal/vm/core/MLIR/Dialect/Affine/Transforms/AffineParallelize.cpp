//===- AffineParallelize.cpp - Affineparallelize Pass---------------------===//
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
// This file implements a parallelizer for affine loop nests that is able to
// perform inner or outer loop parallelization.
//
//===----------------------------------------------------------------------===//

#include "mlir/Dialect/Affine/Transforms/Passes.h"

#include "mlir/Dialect/Affine/Analysis/AffineAnalysis.h"
#include "mlir/Dialect/Affine/Analysis/AffineStructures.h"
#include "mlir/Dialect/Affine/Analysis/LoopAnalysis.h"
#include "mlir/Dialect/Affine/Analysis/Utils.h"
#include "mlir/Dialect/Affine/IR/AffineOps.h"
#include "mlir/Dialect/Affine/LoopUtils.h"
#include "mlir/Dialect/Affine/Transforms/Passes.h.inc"
#include "mlir/Dialect/Affine/Utils.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "vm/core/Support/Debug.h"

namespace mlir {
namespace affine {
#define GEN_PASS_DEF_AFFINEPARALLELIZE
#include "mlir/Dialect/Affine/Transforms/Passes.h.inc"
} // namespace affine
} // namespace mlir

#define DEBUG_TYPE "affine-parallel"

using namespace mlir;
using namespace mlir::affine;

namespace {
/// Convert all parallel affine.for op into 1-D affine.parallel op.
struct AffineParallelize
    : public affine::impl::AffineParallelizeBase<AffineParallelize> {
  using AffineParallelizeBase<AffineParallelize>::AffineParallelizeBase;

  void runOnOperation() override;
};

/// Descriptor of a potentially parallelizable loop.
struct ParallelizationCandidate {
  ParallelizationCandidate(AffineForOp l, SmallVector<LoopReduction> &&r)
      : loop(l), reductions(std::move(r)) {}

  /// The potentially parallelizable loop.
  AffineForOp loop;
  /// Desciprtors of reductions that can be parallelized in the loop.
  SmallVector<LoopReduction> reductions;
};
} // namespace

void AffineParallelize::runOnOperation() {
  func::FuncOp f = getOperation();

  // The walker proceeds in pre-order to process the outer loops first
  // and control the number of outer parallel loops.
  std::vector<ParallelizationCandidate> parallelizableLoops;
  f.walk<WalkOrder::PreOrder>([&](AffineForOp loop) {
    SmallVector<LoopReduction> reductions;
    if (isLoopParallel(loop, parallelReductions ? &reductions : nullptr))
      parallelizableLoops.emplace_back(loop, std::move(reductions));
  });

  for (const ParallelizationCandidate &candidate : parallelizableLoops) {
    unsigned numParentParallelOps = 0;
    AffineForOp loop = candidate.loop;
    for (Operation *op = loop->getParentOp();
         op != nullptr && !op->hasTrait<OpTrait::AffineScope>();
         op = op->getParentOp()) {
      if (isa<AffineParallelOp>(op))
        ++numParentParallelOps;
    }

    if (numParentParallelOps < maxNested) {
      if (failed(affineParallelize(loop, candidate.reductions))) {
        LLVM_DEBUG(toolchain::dbgs() << "[" DEBUG_TYPE "] failed to parallelize\n"
                                << loop);
      }
    } else {
      LLVM_DEBUG(toolchain::dbgs() << "[" DEBUG_TYPE "] too many nested loops\n"
                              << loop);
    }
  }
}
