//===- TargetToDataLayout.cpp - extract data layout from TargetMachine ----===//
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

#include "mlir/Target/LLVMIR/Transforms/Passes.h"
#include "mlir/Target/LLVMIR/Transforms/TargetUtils.h"

#include "mlir/Dialect/DLTI/DLTI.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/Target/LLVMIR/Import.h"

namespace mlir {
namespace LLVM {
#define GEN_PASS_DEF_LLVMTARGETTODATALAYOUT
#include "mlir/Target/LLVMIR/Transforms/Passes.h.inc"
} // namespace LLVM
} // namespace mlir

using namespace mlir;

struct TargetToDataLayoutPass
    : public LLVM::impl::LLVMTargetToDataLayoutBase<TargetToDataLayoutPass> {
  using LLVM::impl::LLVMTargetToDataLayoutBase<
      TargetToDataLayoutPass>::LLVMTargetToDataLayoutBase;

  void runOnOperation() override {
    Operation *op = getOperation();

    if (initializeLLVMTargets)
      LLVM::detail::initializeBackendsOnce();

    auto targetAttr = op->getAttrOfType<LLVM::TargetAttrInterface>(
        LLVM::LLVMDialect::getTargetAttrName());
    if (!targetAttr) {
      op->emitError()
          << "no TargetAttrInterface-implementing attribute at key \""
          << LLVM::LLVMDialect::getTargetAttrName() << "\"";
      return signalPassFailure();
    }

    FailureOr<toolchain::DataLayout> dataLayout =
        LLVM::detail::getDataLayout(targetAttr);
    if (failed(dataLayout)) {
      op->emitError() << "failed to obtain toolchain::DataLayout for " << targetAttr;
      return signalPassFailure();
    }

    DataLayoutSpecInterface dataLayoutSpec =
        mlir::translateDataLayout(dataLayout.value(), &getContext());

    if (auto existingDlSpec = op->getAttrOfType<DataLayoutSpecInterface>(
            DLTIDialect::kDataLayoutAttrName)) {
      dataLayoutSpec = existingDlSpec.combineWith({dataLayoutSpec});
    }

    op->setAttr(DLTIDialect::kDataLayoutAttrName, dataLayoutSpec);
  }
};
