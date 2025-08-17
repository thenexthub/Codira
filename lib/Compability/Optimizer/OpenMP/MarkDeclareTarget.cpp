//===- MarkDeclareTarget.cpp -------------------------------------------===//
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
// Mark functions called from explicit target code as implicitly declare target.
//
//===----------------------------------------------------------------------===//

#include "language/Compability/Optimizer/OpenMP/Passes.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/Dialect/OpenMP/OpenMPDialect.h"
#include "mlir/IR/BuiltinDialect.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/Operation.h"
#include "mlir/IR/SymbolTable.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Support/LLVM.h"
#include "toolchain/ADT/SmallPtrSet.h"

namespace flangomp {
#define GEN_PASS_DEF_MARKDECLARETARGETPASS
#include "language/Compability/Optimizer/OpenMP/Passes.h.inc"
} // namespace flangomp

namespace {
class MarkDeclareTargetPass
    : public flangomp::impl::MarkDeclareTargetPassBase<MarkDeclareTargetPass> {

  void markNestedFuncs(mlir::omp::DeclareTargetDeviceType parentDevTy,
                       mlir::omp::DeclareTargetCaptureClause parentCapClause,
                       bool parentAutomap, mlir::Operation *currOp,
                       toolchain::SmallPtrSet<mlir::Operation *, 16> visited) {
    if (visited.contains(currOp))
      return;
    visited.insert(currOp);

    currOp->walk([&, this](mlir::Operation *op) {
      if (auto callOp = toolchain::dyn_cast<mlir::CallOpInterface>(op)) {
        if (auto symRef = toolchain::dyn_cast_if_present<mlir::SymbolRefAttr>(
                callOp.getCallableForCallee())) {
          if (auto currFOp =
                  getOperation().lookupSymbol<mlir::func::FuncOp>(symRef)) {
            auto current = toolchain::dyn_cast<mlir::omp::DeclareTargetInterface>(
                currFOp.getOperation());

            if (current.isDeclareTarget()) {
              auto currentDt = current.getDeclareTargetDeviceType();

              // Found the same function twice, with different device_types,
              // mark as Any as it belongs to both
              if (currentDt != parentDevTy &&
                  currentDt != mlir::omp::DeclareTargetDeviceType::any) {
                current.setDeclareTarget(
                    mlir::omp::DeclareTargetDeviceType::any,
                    current.getDeclareTargetCaptureClause(),
                    current.getDeclareTargetAutomap());
              }
            } else {
              current.setDeclareTarget(parentDevTy, parentCapClause,
                                       parentAutomap);
            }

            markNestedFuncs(parentDevTy, parentCapClause, parentAutomap,
                            currFOp, visited);
          }
        }
      }
    });
  }

  // This pass executes on mlir::ModuleOp's marking functions contained within
  // as implicitly declare target if they are called from within an explicitly
  // marked declare target function or a target region (TargetOp)
  void runOnOperation() override {
    for (auto functionOp : getOperation().getOps<mlir::func::FuncOp>()) {
      auto declareTargetOp = toolchain::dyn_cast<mlir::omp::DeclareTargetInterface>(
          functionOp.getOperation());
      if (declareTargetOp.isDeclareTarget()) {
        toolchain::SmallPtrSet<mlir::Operation *, 16> visited;
        markNestedFuncs(declareTargetOp.getDeclareTargetDeviceType(),
                        declareTargetOp.getDeclareTargetCaptureClause(),
                        declareTargetOp.getDeclareTargetAutomap(), functionOp,
                        visited);
      }
    }

    // TODO: Extend to work with reverse-offloading, this shouldn't
    // require too much effort, just need to check the device clause
    // when it's lowering has been implemented and change the
    // DeclareTargetDeviceType argument from nohost to host depending on
    // the contents of the device clause
    getOperation()->walk([&](mlir::omp::TargetOp tarOp) {
      toolchain::SmallPtrSet<mlir::Operation *, 16> visited;
      markNestedFuncs(
          /*parentDevTy=*/mlir::omp::DeclareTargetDeviceType::nohost,
          /*parentCapClause=*/mlir::omp::DeclareTargetCaptureClause::to,
          /*parentAutomap=*/false, tarOp, visited);
    });
  }
};

} // namespace
