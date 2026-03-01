//===- InlinerExtension.cpp - Func Inliner Extension ----------------------===//
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

#include "mlir/Dialect/Func/Extensions/InlinerExtension.h"
#include "mlir/Dialect/ControlFlow/IR/ControlFlowOps.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/IR/DialectInterface.h"
#include "mlir/Transforms/InliningUtils.h"

using namespace mlir;
using namespace mlir::func;

//===----------------------------------------------------------------------===//
// FuncDialect Interfaces
//===----------------------------------------------------------------------===//
namespace {
/// This class defines the interface for handling inlining with func operations.
struct FuncInlinerInterface : public DialectInlinerInterface {
  using DialectInlinerInterface::DialectInlinerInterface;

  //===--------------------------------------------------------------------===//
  // Analysis Hooks
  //===--------------------------------------------------------------------===//

  /// Call operations can be inlined unless specified otherwise by attributes
  /// on either the call or the callbale.
  bool isLegalToInline(Operation *call, Operation *callable,
                       bool wouldBeCloned) const final {
    auto callOp = dyn_cast<func::CallOp>(call);
    auto funcOp = dyn_cast<func::FuncOp>(callable);
    return !(callOp && callOp.getNoInline()) &&
           !(funcOp && funcOp.getNoInline());
  }

  /// All operations can be inlined.
  bool isLegalToInline(Operation *, Region *, bool, IRMapping &) const final {
    return true;
  }

  /// All function bodies can be inlined.
  bool isLegalToInline(Region *, Region *, bool, IRMapping &) const final {
    return true;
  }

  //===--------------------------------------------------------------------===//
  // Transformation Hooks
  //===--------------------------------------------------------------------===//

  /// Handle the given inlined terminator by replacing it with a new operation
  /// as necessary.
  void handleTerminator(Operation *op, Block *newDest) const final {
    // Only return needs to be handled here.
    auto returnOp = dyn_cast<ReturnOp>(op);
    if (!returnOp)
      return;

    // Replace the return with a branch to the dest.
    OpBuilder builder(op);
    cf::BranchOp::create(builder, op->getLoc(), newDest,
                         returnOp.getOperands());
    op->erase();
  }

  /// Handle the given inlined terminator by replacing it with a new operation
  /// as necessary.
  void handleTerminator(Operation *op, ValueRange valuesToRepl) const final {
    // Only return needs to be handled here.
    auto returnOp = cast<ReturnOp>(op);

    // Replace the values directly with the return operands.
    assert(returnOp.getNumOperands() == valuesToRepl.size());
    for (const auto &it : toolchain::enumerate(returnOp.getOperands()))
      valuesToRepl[it.index()].replaceAllUsesWith(it.value());
  }
};
} // namespace

//===----------------------------------------------------------------------===//
// Registration
//===----------------------------------------------------------------------===//

void mlir::func::registerInlinerExtension(DialectRegistry &registry) {
  registry.addExtension(+[](MLIRContext *ctx, func::FuncDialect *dialect) {
    dialect->addInterfaces<FuncInlinerInterface>();

    // The inliner extension relies on the ControlFlow dialect.
    ctx->getOrLoadDialect<cf::ControlFlowDialect>();
  });
}
