//===- AffineScalarReplacement.cpp - Affine scalar replacement pass -------===//
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
// This file implements a pass to forward affine memref stores to loads, thereby
// potentially getting rid of intermediate memrefs entirely. It also removes
// redundant loads.
//===----------------------------------------------------------------------===//

#include "mlir/Dialect/Affine/Transforms/Passes.h"

#include "mlir/Analysis/AliasAnalysis.h"
#include "mlir/Dialect/Affine/Utils.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/IR/Dominance.h"

namespace mlir {
namespace affine {
#define GEN_PASS_DEF_AFFINESCALARREPLACEMENT
#include "mlir/Dialect/Affine/Transforms/Passes.h.inc"
} // namespace affine
} // namespace mlir

#define DEBUG_TYPE "affine-scalrep"

using namespace mlir;
using namespace mlir::affine;

namespace {
struct AffineScalarReplacement
    : public affine::impl::AffineScalarReplacementBase<
          AffineScalarReplacement> {
  void runOnOperation() override;
};

} // namespace

std::unique_ptr<OperationPass<func::FuncOp>>
mlir::affine::createAffineScalarReplacementPass() {
  return std::make_unique<AffineScalarReplacement>();
}

void AffineScalarReplacement::runOnOperation() {
  affineScalarReplace(getOperation(), getAnalysis<DominanceInfo>(),
                      getAnalysis<PostDominanceInfo>(),
                      getAnalysis<AliasAnalysis>());
}
