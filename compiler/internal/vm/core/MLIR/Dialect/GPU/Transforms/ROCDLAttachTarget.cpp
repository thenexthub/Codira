//===- ROCDLAttachTarget.cpp - Attach an ROCDL target ---------------------===//
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
// This file implements the `GpuROCDLAttachTarget` pass, attaching
// `#rocdl.target` attributes to GPU modules.
//
//===----------------------------------------------------------------------===//

#include "mlir/Dialect/GPU/Transforms/Passes.h"

#include "mlir/Dialect/GPU/IR/GPUDialect.h"
#include "mlir/Dialect/LLVMIR/ROCDLDialect.h"
#include "mlir/IR/Builders.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Target/LLVM/ROCDL/Target.h"
#include "vm/core/Support/Regex.h"

namespace mlir {
#define GEN_PASS_DEF_GPUROCDLATTACHTARGET
#include "mlir/Dialect/GPU/Transforms/Passes.h.inc"
} // namespace mlir

using namespace mlir;
using namespace mlir::ROCDL;

namespace {
struct ROCDLAttachTarget
    : public impl::GpuROCDLAttachTargetBase<ROCDLAttachTarget> {
  using Base::Base;

  DictionaryAttr getFlags(OpBuilder &builder) const;

  void runOnOperation() override;

  void getDependentDialects(DialectRegistry &registry) const override {
    registry.insert<ROCDL::ROCDLDialect>();
  }
};
} // namespace

DictionaryAttr ROCDLAttachTarget::getFlags(OpBuilder &builder) const {
  UnitAttr unitAttr = builder.getUnitAttr();
  SmallVector<NamedAttribute, 6> flags;
  auto addFlag = [&](StringRef flag) {
    flags.push_back(builder.getNamedAttr(flag, unitAttr));
  };
  if (!wave64Flag)
    addFlag("no_wave64");
  if (fastFlag)
    addFlag("fast");
  if (dazFlag)
    addFlag("daz");
  if (finiteOnlyFlag)
    addFlag("finite_only");
  if (unsafeMathFlag)
    addFlag("unsafe_math");
  if (!correctSqrtFlag)
    addFlag("unsafe_sqrt");
  if (!flags.empty())
    return builder.getDictionaryAttr(flags);
  return nullptr;
}

void ROCDLAttachTarget::runOnOperation() {
  OpBuilder builder(&getContext());
  ArrayRef<std::string> libs(linkLibs);
  SmallVector<StringRef> filesToLink(libs);
  auto target = builder.getAttr<ROCDLTargetAttr>(
      optLevel, triple, chip, features, abiVersion, getFlags(builder),
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
