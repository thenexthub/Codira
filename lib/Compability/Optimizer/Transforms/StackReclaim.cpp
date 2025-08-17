//===- StackReclaim.cpp -- Insert stacksave/stackrestore in region --------===//
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

#include "language/Compability/Optimizer/Builder/FIRBuilder.h"
#include "language/Compability/Optimizer/Dialect/FIRDialect.h"
#include "language/Compability/Optimizer/Dialect/FIROps.h"
#include "language/Compability/Optimizer/Transforms/Passes.h"
#include "language/Compability/Support/Fortran.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/IR/Matchers.h"
#include "mlir/Pass/Pass.h"

namespace fir {
#define GEN_PASS_DEF_STACKRECLAIM
#include "language/Compability/Optimizer/Transforms/Passes.h.inc"
} // namespace fir

using namespace mlir;

namespace {

class StackReclaimPass : public fir::impl::StackReclaimBase<StackReclaimPass> {
public:
  using StackReclaimBase<StackReclaimPass>::StackReclaimBase;

  void runOnOperation() override;
};
} // namespace

void StackReclaimPass::runOnOperation() {
  auto *op = getOperation();
  fir::FirOpBuilder builder(op, fir::getKindMapping(op));

  op->walk([&](fir::DoLoopOp loopOp) {
    mlir::Location loc = loopOp.getLoc();

    if (!loopOp.getRegion().getOps<fir::AllocaOp>().empty()) {
      builder.setInsertionPointToStart(&loopOp.getRegion().front());
      mlir::Value sp = builder.genStackSave(loc);

      auto *terminator = loopOp.getRegion().back().getTerminator();
      builder.setInsertionPoint(terminator);
      builder.genStackRestore(loc, sp);
    }
  });
}
