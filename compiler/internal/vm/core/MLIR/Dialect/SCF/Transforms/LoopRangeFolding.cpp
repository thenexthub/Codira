//===- LoopRangeFolding.cpp - Code to perform loop range folding-----------===//
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
// This file implements loop range folding.
//
//===----------------------------------------------------------------------===//

#include "mlir/Dialect/SCF/Transforms/Passes.h"

#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/Dialect/SCF/Transforms/Transforms.h"
#include "mlir/Dialect/SCF/Utils/Utils.h"
#include "mlir/IR/IRMapping.h"

namespace mlir {
#define GEN_PASS_DEF_SCFFORLOOPRANGEFOLDING
#include "mlir/Dialect/SCF/Transforms/Passes.h.inc"
} // namespace mlir

using namespace mlir;
using namespace mlir::scf;

namespace {
struct ForLoopRangeFolding
    : public impl::SCFForLoopRangeFoldingBase<ForLoopRangeFolding> {
  void runOnOperation() override;
};
} // namespace

void ForLoopRangeFolding::runOnOperation() {
  getOperation()->walk([&](ForOp op) {
    Value indVar = op.getInductionVar();

    auto canBeFolded = [&](Value value) {
      return op.isDefinedOutsideOfLoop(value) || value == indVar;
    };

    // Fold until a fixed point is reached
    while (true) {

      // If the induction variable is used more than once, we can't fold its
      // arith ops into the loop range
      if (!indVar.hasOneUse())
        break;

      Operation *user = *indVar.getUsers().begin();
      if (!isa<arith::AddIOp, arith::MulIOp>(user))
        break;

      if (!toolchain::all_of(user->getOperands(), canBeFolded))
        break;

      OpBuilder b(op);
      IRMapping lbMap;
      lbMap.map(indVar, op.getLowerBound());
      IRMapping ubMap;
      ubMap.map(indVar, op.getUpperBound());
      IRMapping stepMap;
      stepMap.map(indVar, op.getStep());

      if (isa<arith::AddIOp>(user)) {
        Operation *lbFold = b.clone(*user, lbMap);
        Operation *ubFold = b.clone(*user, ubMap);

        op.setLowerBound(lbFold->getResult(0));
        op.setUpperBound(ubFold->getResult(0));

      } else if (isa<arith::MulIOp>(user)) {
        Operation *lbFold = b.clone(*user, lbMap);
        Operation *ubFold = b.clone(*user, ubMap);
        Operation *stepFold = b.clone(*user, stepMap);

        op.setLowerBound(lbFold->getResult(0));
        op.setUpperBound(ubFold->getResult(0));
        op.setStep(stepFold->getResult(0));
      }

      ValueRange wrapIndvar(indVar);
      user->replaceAllUsesWith(wrapIndvar);
      user->erase();
    }
  });
}

std::unique_ptr<Pass> mlir::createForLoopRangeFoldingPass() {
  return std::make_unique<ForLoopRangeFolding>();
}
