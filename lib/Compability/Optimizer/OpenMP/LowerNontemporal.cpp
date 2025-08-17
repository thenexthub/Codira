//===- LowerNontemporal.cpp -------------------------------------------===//
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
//
// Add nontemporal attributes to load and stores of variables marked as
// nontemporal.
//
//===----------------------------------------------------------------------===//

#include "language/Compability/Optimizer/Dialect/FIRCG/CGOps.h"
#include "language/Compability/Optimizer/Dialect/FIROpsSupport.h"
#include "language/Compability/Optimizer/OpenMP/Passes.h"
#include "mlir/Dialect/OpenMP/OpenMPDialect.h"
#include "toolchain/ADT/TypeSwitch.h"

using namespace mlir;

namespace flangomp {
#define GEN_PASS_DEF_LOWERNONTEMPORALPASS
#include "language/Compability/Optimizer/OpenMP/Passes.h.inc"
} // namespace flangomp

namespace {
class LowerNontemporalPass
    : public flangomp::impl::LowerNontemporalPassBase<LowerNontemporalPass> {
  void addNonTemporalAttr(omp::SimdOp simdOp) {
    if (simdOp.getNontemporalVars().empty())
      return;

    std::function<mlir::Value(mlir::Value)> getBaseOperand =
        [&](mlir::Value operand) -> mlir::Value {
      auto *defOp = operand.getDefiningOp();
      while (defOp) {
        toolchain::TypeSwitch<Operation *>(defOp)
            .Case<fir::ArrayCoorOp, fir::cg::XArrayCoorOp, fir::LoadOp>(
                [&](auto op) {
                  operand = op.getMemref();
                  defOp = operand.getDefiningOp();
                })
            .Case<fir::BoxAddrOp>([&](auto op) {
              operand = op.getVal();
              defOp = operand.getDefiningOp();
            })
            .Default([&](auto op) { defOp = nullptr; });
      }
      return operand;
    };

    // walk through the operations and mark the load and store as nontemporal
    simdOp->walk([&](Operation *op) {
      mlir::Value operand = nullptr;

      if (auto loadOp = toolchain::dyn_cast<fir::LoadOp>(op))
        operand = loadOp.getMemref();
      else if (auto storeOp = toolchain::dyn_cast<fir::StoreOp>(op))
        operand = storeOp.getMemref();

      // Skip load and store operations involving boxes (allocatable or pointer
      // types).
      if (operand && !(fir::isAllocatableType(operand.getType()) ||
                       fir::isPointerType((operand.getType())))) {
        operand = getBaseOperand(operand);

        // TODO : Handling of nontemporal clause inside atomic construct
        if (toolchain::is_contained(simdOp.getNontemporalVars(), operand)) {
          if (auto loadOp = toolchain::dyn_cast<fir::LoadOp>(op))
            loadOp.setNontemporal(true);
          else if (auto storeOp = toolchain::dyn_cast<fir::StoreOp>(op))
            storeOp.setNontemporal(true);
        }
      }
    });
  }

  void runOnOperation() override {
    Operation *op = getOperation();
    op->walk([&](omp::SimdOp simdOp) { addNonTemporalAttr(simdOp); });
  }
};
} // namespace
