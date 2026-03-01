//===- Target.cpp - MLIR SPIR-V target compilation --------------*- C++ -*-===//
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
// This files defines SPIR-V target related functions including registration
// calls for the `#spirv.target_env` compilation attribute.
//
//===----------------------------------------------------------------------===//

#include "mlir/Target/SPIRV/Target.h"

#include "mlir/Dialect/GPU/IR/GPUDialect.h"
#include "mlir/Dialect/SPIRV/IR/SPIRVAttributes.h"
#include "mlir/Dialect/SPIRV/IR/SPIRVDialect.h"
#include "mlir/Dialect/SPIRV/IR/SPIRVOps.h"
#include "mlir/Target/SPIRV/Serialization.h"

#include <cstdlib>
#include <cstring>

using namespace mlir;
using namespace mlir::spirv;

namespace {
// SPIR-V implementation of the gpu:TargetAttrInterface.
class SPIRVTargetAttrImpl
    : public gpu::TargetAttrInterface::FallbackModel<SPIRVTargetAttrImpl> {
public:
  std::optional<SmallVector<char, 0>>
  serializeToObject(Attribute attribute, Operation *module,
                    const gpu::TargetOptions &options) const;

  Attribute createObject(Attribute attribute, Operation *module,
                         const SmallVector<char, 0> &object,
                         const gpu::TargetOptions &options) const;
};
} // namespace

// Register the SPIR-V dialect, the SPIR-V translation & the target interface.
void mlir::spirv::registerSPIRVTargetInterfaceExternalModels(
    DialectRegistry &registry) {
  registry.addExtension(+[](MLIRContext *ctx, spirv::SPIRVDialect *dialect) {
    spirv::TargetEnvAttr::attachInterface<SPIRVTargetAttrImpl>(*ctx);
  });
}

void mlir::spirv::registerSPIRVTargetInterfaceExternalModels(
    MLIRContext &context) {
  DialectRegistry registry;
  registerSPIRVTargetInterfaceExternalModels(registry);
  context.appendDialectRegistry(registry);
}

// Reuse from existing serializer
std::optional<SmallVector<char, 0>> SPIRVTargetAttrImpl::serializeToObject(
    Attribute attribute, Operation *module,
    const gpu::TargetOptions &options) const {
  if (!module)
    return std::nullopt;
  auto gpuMod = dyn_cast<gpu::GPUModuleOp>(module);
  if (!gpuMod) {
    module->emitError("expected to be a gpu.module op");
    return std::nullopt;
  }
  auto spvMods = gpuMod.getOps<spirv::ModuleOp>();
  if (spvMods.empty())
    return std::nullopt;

  auto spvMod = *spvMods.begin();
  toolchain::SmallVector<uint32_t, 0> spvBinary;

  spvBinary.clear();
  // Serialize the spirv.module op to SPIR-V blob.
  if (mlir::failed(spirv::serialize(spvMod, spvBinary))) {
    spvMod.emitError() << "failed to serialize SPIR-V module";
    return std::nullopt;
  }

  SmallVector<char, 0> spvData(spvBinary.size() * sizeof(uint32_t), 0);
  std::memcpy(spvData.data(), spvBinary.data(), spvData.size());

  spvMod.erase();
  return spvData;
}

// Prepare Attribute for gpu.binary with serialized kernel object
Attribute
SPIRVTargetAttrImpl::createObject(Attribute attribute, Operation *module,
                                  const SmallVector<char, 0> &object,
                                  const gpu::TargetOptions &options) const {
  gpu::CompilationTarget format = options.getCompilationTarget();
  DictionaryAttr objectProps;
  Builder builder(attribute.getContext());
  return builder.getAttr<gpu::ObjectAttr>(
      attribute, format,
      builder.getStringAttr(StringRef(object.data(), object.size())),
      objectProps, /*kernels=*/nullptr);
}
