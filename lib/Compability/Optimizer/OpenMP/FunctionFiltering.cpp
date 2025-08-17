//===- FunctionFiltering.cpp -------------------------------------------===//
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
// This file implements transforms to filter out functions intended for the host
// when compiling for the device and vice versa.
//
//===----------------------------------------------------------------------===//

#include "language/Compability/Optimizer/Dialect/FIRDialect.h"
#include "language/Compability/Optimizer/Dialect/FIROpsSupport.h"
#include "language/Compability/Optimizer/OpenMP/Passes.h"

#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/OpenMP/OpenMPDialect.h"
#include "mlir/Dialect/OpenMP/OpenMPInterfaces.h"
#include "mlir/IR/BuiltinOps.h"
#include "toolchain/ADT/SmallVector.h"

namespace flangomp {
#define GEN_PASS_DEF_FUNCTIONFILTERINGPASS
#include "language/Compability/Optimizer/OpenMP/Passes.h.inc"
} // namespace flangomp

using namespace mlir;

namespace {
class FunctionFilteringPass
    : public flangomp::impl::FunctionFilteringPassBase<FunctionFilteringPass> {
public:
  FunctionFilteringPass() = default;

  void runOnOperation() override {
    MLIRContext *context = &getContext();
    OpBuilder opBuilder(context);
    auto op = dyn_cast<omp::OffloadModuleInterface>(getOperation());
    if (!op || !op.getIsTargetDevice())
      return;

    op->walk<WalkOrder::PreOrder>([&](func::FuncOp funcOp) {
      // Do not filter functions with target regions inside, because they have
      // to be available for both host and device so that regular and reverse
      // offloading can be supported.
      bool hasTargetRegion =
          funcOp
              ->walk<WalkOrder::PreOrder>(
                  [&](omp::TargetOp) { return WalkResult::interrupt(); })
              .wasInterrupted();

      omp::DeclareTargetDeviceType declareType =
          omp::DeclareTargetDeviceType::host;
      auto declareTargetOp =
          dyn_cast<omp::DeclareTargetInterface>(funcOp.getOperation());
      if (declareTargetOp && declareTargetOp.isDeclareTarget())
        declareType = declareTargetOp.getDeclareTargetDeviceType();

      // Filtering a function here means deleting it if it doesn't contain a
      // target region. Else we explicitly set the omp.declare_target
      // attribute. The second stage of function filtering at the MLIR to LLVM
      // IR translation level will remove functions that contain the target
      // region from the generated toolchain IR.
      if (declareType == omp::DeclareTargetDeviceType::host) {
        SymbolTable::UseRange funcUses = *funcOp.getSymbolUses(op);
        for (SymbolTable::SymbolUse use : funcUses) {
          Operation *callOp = use.getUser();
          if (auto internalFunc = mlir::dyn_cast<func::FuncOp>(callOp)) {
            // Do not delete internal procedures holding the symbol of their
            // Fortran host procedure as attribute.
            internalFunc->removeAttr(fir::getHostSymbolAttrName());
            // Set public visibility so that the function is not deleted by MLIR
            // because unused. Changing it is OK here because the function will
            // be deleted anyway in the second filtering phase.
            internalFunc.setVisibility(mlir::SymbolTable::Visibility::Public);
            continue;
          }
          // If the callOp has users then replace them with Undef values.
          if (!callOp->use_empty()) {
            SmallVector<Value> undefResults;
            for (Value res : callOp->getResults()) {
              opBuilder.setInsertionPoint(callOp);
              undefResults.emplace_back(
                  fir::UndefOp::create(opBuilder, res.getLoc(), res.getType()));
            }
            callOp->replaceAllUsesWith(undefResults);
          }
          // Remove the callOp
          callOp->erase();
        }
        if (!hasTargetRegion) {
          funcOp.erase();
          return WalkResult::skip();
        }
        if (declareTargetOp)
          declareTargetOp.setDeclareTarget(
              declareType, omp::DeclareTargetCaptureClause::to,
              declareTargetOp.getDeclareTargetAutomap());
      }
      return WalkResult::advance();
    });
  }
};
} // namespace
