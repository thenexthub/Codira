//===- TargetToTargetFeatures.cpp - extract features from TargetMachine ---===//
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

#include "vm/core/MC/MCSubtargetInfo.h"

namespace mlir {
namespace LLVM {
#define GEN_PASS_DEF_LLVMTARGETTOTARGETFEATURES
#include "mlir/Target/LLVMIR/Transforms/Passes.h.inc"
} // namespace LLVM
} // namespace mlir

using namespace mlir;

struct TargetToTargetFeaturesPass
    : public LLVM::impl::LLVMTargetToTargetFeaturesBase<
          TargetToTargetFeaturesPass> {
  using LLVM::impl::LLVMTargetToTargetFeaturesBase<
      TargetToTargetFeaturesPass>::LLVMTargetToTargetFeaturesBase;

  void runOnOperation() override {
    Operation *op = getOperation();

    if (initializeLLVMTargets)
      LLVM::detail::initializeBackendsOnce();

    auto targetAttr = op->getAttrOfType<LLVM::TargetAttr>(
        LLVM::LLVMDialect::getTargetAttrName());
    if (!targetAttr) {
      op->emitError() << "no LLVM::TargetAttr attribute at key \""
                      << LLVM::LLVMDialect::getTargetAttrName() << "\"";
      return signalPassFailure();
    }

    FailureOr<std::unique_ptr<toolchain::TargetMachine>> targetMachine =
        LLVM::detail::getTargetMachine(targetAttr);
    if (failed(targetMachine)) {
      op->emitError() << "failed to obtain toolchain::TargetMachine for "
                      << targetAttr;
      return signalPassFailure();
    }

    toolchain::MCSubtargetInfo const *subTargetInfo =
        (*targetMachine)->getMCSubtargetInfo();

    const std::vector<toolchain::SubtargetFeatureKV> enabledFeatures =
        subTargetInfo->getEnabledProcessorFeatures();

    auto plussedFeatures = toolchain::to_vector(
        toolchain::map_range(enabledFeatures, [](toolchain::SubtargetFeatureKV feature) {
          return std::string("+") + feature.Key;
        }));

    auto plussedFeaturesRefs = toolchain::to_vector(toolchain::map_range(
        plussedFeatures, [](auto &it) { return StringRef(it.c_str()); }));

    auto fullTargetFeaturesAttr =
        LLVM::TargetFeaturesAttr::get(&getContext(), plussedFeaturesRefs);

    auto updatedTargetAttr =
        LLVM::TargetAttr::get(&getContext(), targetAttr.getTriple(),
                              targetAttr.getChip(), fullTargetFeaturesAttr);

    op->setAttr(LLVM::LLVMDialect::getTargetAttrName(), updatedTargetAttr);
  }
};
