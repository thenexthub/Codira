//===- LoopCoalescing.cpp - Pass transforming loop nests into single loops-===//
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

#include "mlir/Dialect/Affine/Transforms/Passes.h"

#include "mlir/Dialect/Affine/IR/AffineOps.h"
#include "mlir/Dialect/Affine/LoopUtils.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/Dialect/SCF/Utils/Utils.h"

namespace mlir {
namespace affine {
#define GEN_PASS_DEF_LOOPCOALESCING
#include "mlir/Dialect/Affine/Transforms/Passes.h.inc"
} // namespace affine
} // namespace mlir

#define PASS_NAME "loop-coalescing"
#define DEBUG_TYPE PASS_NAME

using namespace mlir;
using namespace mlir::affine;

namespace {
struct LoopCoalescingPass
    : public affine::impl::LoopCoalescingBase<LoopCoalescingPass> {

  void runOnOperation() override {
    func::FuncOp func = getOperation();
    func.walk<WalkOrder::PreOrder>([](Operation *op) {
      if (auto scfForOp = dyn_cast<scf::ForOp>(op))
        (void)coalescePerfectlyNestedSCFForLoops(scfForOp);
      else if (auto affineForOp = dyn_cast<AffineForOp>(op))
        (void)coalescePerfectlyNestedAffineLoops(affineForOp);
    });
  }
};

} // namespace

std::unique_ptr<OperationPass<func::FuncOp>>
mlir::affine::createLoopCoalescingPass() {
  return std::make_unique<LoopCoalescingPass>();
}
