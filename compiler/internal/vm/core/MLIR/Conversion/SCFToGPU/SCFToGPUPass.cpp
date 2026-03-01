//===- SCFToGPUPass.cpp - Convert a loop nest to a GPU kernel -----------===//
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

#include "mlir/Conversion/SCFToGPU/SCFToGPUPass.h"

#include "mlir/Conversion/SCFToGPU/SCFToGPU.h"
#include "mlir/Dialect/Affine/IR/AffineOps.h"
#include "mlir/Dialect/GPU/IR/GPUDialect.h"
#include "mlir/Transforms/DialectConversion.h"

namespace mlir {
#define GEN_PASS_DEF_CONVERTAFFINEFORTOGPUPASS
#define GEN_PASS_DEF_CONVERTPARALLELLOOPTOGPUPASS
#include "mlir/Conversion/Passes.h.inc"
} // namespace mlir

using namespace mlir;
using namespace mlir::scf;

namespace {
// A pass that traverses top-level loops in the function and converts them to
// GPU launch operations.  Nested launches are not allowed, so this does not
// walk the function recursively to avoid considering nested loops.
struct ForLoopMapper
    : public impl::ConvertAffineForToGPUPassBase<ForLoopMapper> {
  using Base::Base;

  void runOnOperation() override {
    for (Operation &op : toolchain::make_early_inc_range(
             getOperation().getFunctionBody().getOps())) {
      if (auto forOp = dyn_cast<affine::AffineForOp>(&op)) {
        if (failed(convertAffineLoopNestToGPULaunch(forOp, numBlockDims,
                                                    numThreadDims)))
          signalPassFailure();
      }
    }
  }
};

struct ParallelLoopToGpuPass
    : public impl::ConvertParallelLoopToGpuPassBase<ParallelLoopToGpuPass> {
  void runOnOperation() override {
    RewritePatternSet patterns(&getContext());
    populateParallelLoopToGPUPatterns(patterns);
    ConversionTarget target(getContext());
    target.markUnknownOpDynamicallyLegal([](Operation *) { return true; });
    configureParallelLoopToGPULegality(target);
    if (failed(applyPartialConversion(getOperation(), target,
                                      std::move(patterns))))
      signalPassFailure();
    finalizeParallelLoopToGPUConversion(getOperation());
  }
};

} // namespace
