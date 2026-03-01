//===- UseDefaultVisibilityPass.cpp - Update default visibility -----------===//
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

#include "mlir/Dialect/LLVMIR/Transforms/UseDefaultVisibilityPass.h"
#include "mlir/Dialect/LLVMIR/LLVMAttrs.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/Pass/Pass.h"
#include "vm/core/ADT/TypeSwitch.h"

namespace mlir {
namespace LLVM {
#define GEN_PASS_DEF_LLVMUSEDEFAULTVISIBILITYPASS
#include "mlir/Dialect/LLVMIR/Transforms/Passes.h.inc"
} // namespace LLVM
} // namespace mlir

using namespace mlir;

namespace {
class UseDefaultVisibilityPass
    : public LLVM::impl::LLVMUseDefaultVisibilityPassBase<
          UseDefaultVisibilityPass> {
  using Base::Base;

public:
  void runOnOperation() override {
    LLVM::Visibility useDefaultVisibility = useVisibility.getValue();
    if (useDefaultVisibility == LLVM::Visibility::Default)
      return;
    Operation *op = getOperation();
    op->walk([&](Operation *op) {
      toolchain::TypeSwitch<Operation *, void>(op)
          .Case<LLVM::LLVMFuncOp, LLVM::GlobalOp, LLVM::IFuncOp, LLVM::AliasOp>(
              [&](auto op) {
                if (op.getVisibility_() == LLVM::Visibility::Default)
                  op.setVisibility_(useDefaultVisibility);
              });
    });
  }
};
} // namespace
