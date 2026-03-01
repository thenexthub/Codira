//===- SPIRVAttachTarget.cpp - Attach an SPIR-V target --------------------===//
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
// This file implements the `GPUSPIRVAttachTarget` pass, attaching
// `#spirv.target_env` attributes to GPU modules.
//
//===----------------------------------------------------------------------===//

#include "mlir/Dialect/GPU/Transforms/Passes.h"

#include "mlir/Dialect/GPU/IR/GPUDialect.h"
#include "mlir/Dialect/SPIRV/IR/SPIRVAttributes.h"
#include "mlir/Dialect/SPIRV/IR/SPIRVDialect.h"
#include "mlir/Dialect/SPIRV/IR/TargetAndABI.h"
#include "mlir/IR/Builders.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Target/SPIRV/Target.h"
#include "vm/core/Support/Regex.h"

namespace mlir {
#define GEN_PASS_DEF_GPUSPIRVATTACHTARGET
#include "mlir/Dialect/GPU/Transforms/Passes.h.inc"
} // namespace mlir

using namespace mlir;
using namespace mlir::spirv;

namespace {
struct SPIRVAttachTarget
    : public impl::GpuSPIRVAttachTargetBase<SPIRVAttachTarget> {
  using Base::Base;

  void runOnOperation() override;

  void getDependentDialects(DialectRegistry &registry) const override {
    registry.insert<spirv::SPIRVDialect>();
  }
};
} // namespace

void SPIRVAttachTarget::runOnOperation() {
  OpBuilder builder(&getContext());
  auto versionSymbol = symbolizeVersion(spirvVersion);
  if (!versionSymbol)
    return signalPassFailure();
  auto apiSymbol = symbolizeClientAPI(clientApi);
  if (!apiSymbol)
    return signalPassFailure();
  auto vendorSymbol = symbolizeVendor(deviceVendor);
  if (!vendorSymbol)
    return signalPassFailure();
  auto deviceTypeSymbol = symbolizeDeviceType(deviceType);
  if (!deviceTypeSymbol)
    return signalPassFailure();
  // Set the default device ID if none was given
  if (!deviceId.hasValue())
    deviceId = mlir::spirv::TargetEnvAttr::kUnknownDeviceID;

  Version version = versionSymbol.value();
  SmallVector<Capability, 4> capabilities;
  SmallVector<Extension, 8> extensions;
  for (const auto &cap : spirvCapabilities) {
    auto capSymbol = symbolizeCapability(cap);
    if (capSymbol)
      capabilities.push_back(capSymbol.value());
  }
  ArrayRef<Capability> caps(capabilities);
  for (const auto &ext : spirvExtensions) {
    auto extSymbol = symbolizeExtension(ext);
    if (extSymbol)
      extensions.push_back(extSymbol.value());
  }
  ArrayRef<Extension> exts(extensions);
  VerCapExtAttr vce = VerCapExtAttr::get(version, caps, exts, &getContext());
  auto target = TargetEnvAttr::get(vce, getDefaultResourceLimits(&getContext()),
                                   apiSymbol.value(), vendorSymbol.value(),
                                   deviceTypeSymbol.value(), deviceId);
  toolchain::Regex matcher(moduleMatcher);
  getOperation()->walk([&](gpu::GPUModuleOp gpuModule) {
    // Check if the name of the module matches.
    if (!moduleMatcher.empty() && !matcher.match(gpuModule.getName()))
      return;
    // Create the target array.
    SmallVector<Attribute> targets;
    if (std::optional<ArrayAttr> attrs = gpuModule.getTargets())
      targets.append(attrs->getValue().begin(), attrs->getValue().end());
    targets.push_back(target);
    // Remove any duplicate targets.
    targets.erase(toolchain::unique(targets), targets.end());
    // Update the target attribute array.
    gpuModule.setTargetsAttr(builder.getArrayAttr(targets));
  });
}
