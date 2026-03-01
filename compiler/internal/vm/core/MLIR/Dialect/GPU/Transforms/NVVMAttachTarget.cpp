//===- NVVMAttachTarget.cpp - Attach an NVVM target -----------------------===//
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
// This file implements the `GpuNVVMAttachTarget` pass, attaching `#nvvm.target`
// attributes to GPU modules.
//
//===----------------------------------------------------------------------===//

#include "mlir/Dialect/GPU/Transforms/Passes.h"

#include "mlir/Dialect/GPU/IR/GPUDialect.h"
#include "mlir/Dialect/LLVMIR/NVVMDialect.h"
#include "mlir/IR/Builders.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Target/LLVM/NVVM/Target.h"
#include "vm/core/Support/Regex.h"

namespace mlir {
#define GEN_PASS_DEF_GPUNVVMATTACHTARGET
#include "mlir/Dialect/GPU/Transforms/Passes.h.inc"
} // namespace mlir

using namespace mlir;
using namespace mlir::NVVM;

namespace {
struct NVVMAttachTarget
    : public impl::GpuNVVMAttachTargetBase<NVVMAttachTarget> {
  using Base::Base;

  DictionaryAttr getFlags(OpBuilder &builder) const;

  void runOnOperation() override;

  void getDependentDialects(DialectRegistry &registry) const override {
    registry.insert<NVVM::NVVMDialect>();
  }
};
} // namespace

DictionaryAttr NVVMAttachTarget::getFlags(OpBuilder &builder) const {
  UnitAttr unitAttr = builder.getUnitAttr();
  SmallVector<NamedAttribute, 3> flags;
  auto addFlag = [&](StringRef flag) {
    flags.push_back(builder.getNamedAttr(flag, unitAttr));
  };
  if (fastFlag)
    addFlag("fast");
  if (ftzFlag)
    addFlag("ftz");

  // Tokenize and set the optional command line options.
  if (!cmdOptions.empty()) {
    auto options = gpu::TargetOptions::tokenizeCmdOptions(cmdOptions);
    if (!options.second.empty()) {
      toolchain::SmallVector<mlir::Attribute> nvvmOptionAttrs;
      for (const char *opt : options.second) {
        nvvmOptionAttrs.emplace_back(
            mlir::StringAttr::get(builder.getContext(), StringRef(opt)));
      }
      flags.push_back(builder.getNamedAttr(
          "ptxas-cmd-options",
          mlir::ArrayAttr::get(builder.getContext(), nvvmOptionAttrs)));
    }
  }

  if (!flags.empty())
    return builder.getDictionaryAttr(flags);
  return nullptr;
}

void NVVMAttachTarget::runOnOperation() {
  OpBuilder builder(&getContext());
  ArrayRef<std::string> libs(linkLibs);
  SmallVector<StringRef> filesToLink(libs);
  auto target = builder.getAttr<NVVMTargetAttr>(
      optLevel, triple, chip, features, getFlags(builder),
      filesToLink.empty() ? nullptr : builder.getStrArrayAttr(filesToLink));
  toolchain::Regex matcher(moduleMatcher);
  for (Region &region : getOperation()->getRegions())
    for (Block &block : region.getBlocks())
      for (auto module : block.getOps<gpu::GPUModuleOp>()) {
        // Check if the name of the module matches.
        if (!moduleMatcher.empty() && !matcher.match(module.getName()))
          continue;
        // Create the target array.
        SmallVector<Attribute> targets;
        if (std::optional<ArrayAttr> attrs = module.getTargets())
          targets.append(attrs->getValue().begin(), attrs->getValue().end());
        targets.push_back(target);
        // Remove any duplicate targets.
        targets.erase(toolchain::unique(targets), targets.end());
        // Update the target attribute array.
        module.setTargetsAttr(builder.getArrayAttr(targets));
      }
}
