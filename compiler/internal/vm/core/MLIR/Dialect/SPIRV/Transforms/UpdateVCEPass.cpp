//===- DeduceVersionExtensionCapabilityPass.cpp ---------------------------===//
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
// This file implements a pass to deduce minimal version/extension/capability
// requirements for a spirv::ModuleOp.
//
//===----------------------------------------------------------------------===//

#include "mlir/Dialect/SPIRV/Transforms/Passes.h"

#include "mlir/Dialect/SPIRV/IR/SPIRVOps.h"
#include "mlir/Dialect/SPIRV/IR/SPIRVTypes.h"
#include "mlir/Dialect/SPIRV/IR/TargetAndABI.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/Visitors.h"
#include "vm/core/ADT/StringExtras.h"
#include <optional>

namespace mlir {
namespace spirv {
#define GEN_PASS_DEF_SPIRVUPDATEVCEPASS
#include "mlir/Dialect/SPIRV/Transforms/Passes.h.inc"
} // namespace spirv
} // namespace mlir

using namespace mlir;

namespace {
/// Pass to deduce minimal version/extension/capability requirements for a
/// spirv::ModuleOp.
class UpdateVCEPass final
    : public spirv::impl::SPIRVUpdateVCEPassBase<UpdateVCEPass> {
  void runOnOperation() override;
};
} // namespace

/// Checks that `candidates` extension requirements are possible to be satisfied
/// with the given `targetEnv` and updates `deducedExtensions` if so. Emits
/// errors attaching to the given `op` on failures.
///
///  `candidates` is a vector of vector for extension requirements following
/// ((Extension::A OR Extension::B) AND (Extension::C OR Extension::D))
/// convention.
static LogicalResult checkAndUpdateExtensionRequirements(
    Operation *op, const spirv::TargetEnv &targetEnv,
    const spirv::SPIRVType::ExtensionArrayRefVector &candidates,
    SetVector<spirv::Extension> &deducedExtensions) {
  for (const auto &ors : candidates) {
    if (std::optional<spirv::Extension> chosen = targetEnv.allows(ors)) {
      deducedExtensions.insert(*chosen);
    } else {
      SmallVector<StringRef, 4> extStrings;
      for (spirv::Extension ext : ors)
        extStrings.push_back(spirv::stringifyExtension(ext));

      return op->emitError("'")
             << op->getName() << "' requires at least one extension in ["
             << toolchain::join(extStrings, ", ")
             << "] but none allowed in target environment";
    }
  }
  return success();
}

/// Checks that `candidates`capability requirements are possible to be satisfied
/// with the given `targetEnv` and updates `deducedCapabilities` if so. Emits
/// errors attaching to the given `op` on failures.
///
///  `candidates` is a vector of vector for capability requirements following
/// ((Capability::A OR Capability::B) AND (Capability::C OR Capability::D))
/// convention.
static LogicalResult checkAndUpdateCapabilityRequirements(
    Operation *op, const spirv::TargetEnv &targetEnv,
    const spirv::SPIRVType::CapabilityArrayRefVector &candidates,
    SetVector<spirv::Capability> &deducedCapabilities) {
  for (const auto &ors : candidates) {
    if (std::optional<spirv::Capability> chosen = targetEnv.allows(ors)) {
      deducedCapabilities.insert(*chosen);
    } else {
      SmallVector<StringRef, 4> capStrings;
      for (spirv::Capability cap : ors)
        capStrings.push_back(spirv::stringifyCapability(cap));

      return op->emitError("'")
             << op->getName() << "' requires at least one capability in ["
             << toolchain::join(capStrings, ", ")
             << "] but none allowed in target environment";
    }
  }
  return success();
}

static void addAllImpliedCapabilities(SetVector<spirv::Capability> &caps) {
  SetVector<spirv::Capability> tmp;
  for (spirv::Capability cap : caps)
    tmp.insert_range(getRecursiveImpliedCapabilities(cap));
  caps.insert_range(std::move(tmp));
}

void UpdateVCEPass::runOnOperation() {
  spirv::ModuleOp module = getOperation();

  spirv::TargetEnvAttr targetAttr = spirv::lookupTargetEnv(module);
  if (!targetAttr) {
    module.emitError("missing 'spirv.target_env' attribute");
    return signalPassFailure();
  }

  spirv::TargetEnv targetEnv(targetAttr);
  spirv::Version allowedVersion = targetAttr.getVersion();

  spirv::Version deducedVersion = spirv::Version::V_1_0;
  SetVector<spirv::Extension> deducedExtensions;
  SetVector<spirv::Capability> deducedCapabilities;

  // Walk each SPIR-V op to deduce the minimal version/extension/capability
  // requirements.
  WalkResult walkResult = module.walk([&](Operation *op) -> WalkResult {
    // Op min version requirements
    if (auto minVersionIfx = dyn_cast<spirv::QueryMinVersionInterface>(op)) {
      std::optional<spirv::Version> minVersion = minVersionIfx.getMinVersion();
      if (minVersion) {
        deducedVersion = std::max(deducedVersion, *minVersion);
        if (deducedVersion > allowedVersion) {
          return op->emitError("'")
                 << op->getName() << "' requires min version "
                 << spirv::stringifyVersion(deducedVersion)
                 << " but target environment allows up to "
                 << spirv::stringifyVersion(allowedVersion);
        }
      }
    }

    // Op extension requirements
    if (auto extensions = dyn_cast<spirv::QueryExtensionInterface>(op))
      if (failed(checkAndUpdateExtensionRequirements(
              op, targetEnv, extensions.getExtensions(), deducedExtensions)))
        return WalkResult::interrupt();

    // Op capability requirements
    if (auto capabilities = dyn_cast<spirv::QueryCapabilityInterface>(op))
      if (failed(checkAndUpdateCapabilityRequirements(
              op, targetEnv, capabilities.getCapabilities(),
              deducedCapabilities)))
        return WalkResult::interrupt();

    SmallVector<Type, 4> valueTypes;
    valueTypes.append(op->operand_type_begin(), op->operand_type_end());
    valueTypes.append(op->result_type_begin(), op->result_type_end());

    // Special treatment for global variables, whose type requirements are
    // conveyed by type attributes.
    if (auto globalVar = dyn_cast<spirv::GlobalVariableOp>(op))
      valueTypes.push_back(globalVar.getType());

    // If the op is FunctionLike make sure to process input and result types.
    if (auto funcOpInterface = dyn_cast<FunctionOpInterface>(op)) {
      toolchain::append_range(valueTypes, funcOpInterface.getArgumentTypes());
      toolchain::append_range(valueTypes, funcOpInterface.getResultTypes());
    }

    // Requirements from values' types
    SmallVector<ArrayRef<spirv::Extension>, 4> typeExtensions;
    SmallVector<ArrayRef<spirv::Capability>, 8> typeCapabilities;
    for (Type valueType : valueTypes) {
      typeExtensions.clear();
      cast<spirv::SPIRVType>(valueType).getExtensions(typeExtensions);
      if (failed(checkAndUpdateExtensionRequirements(
              op, targetEnv, typeExtensions, deducedExtensions)))
        return WalkResult::interrupt();

      typeCapabilities.clear();
      cast<spirv::SPIRVType>(valueType).getCapabilities(typeCapabilities);
      if (failed(checkAndUpdateCapabilityRequirements(
              op, targetEnv, typeCapabilities, deducedCapabilities)))
        return WalkResult::interrupt();
    }

    return WalkResult::advance();
  });

  if (walkResult.wasInterrupted())
    return signalPassFailure();

  addAllImpliedCapabilities(deducedCapabilities);

  // Update min version requirement for capabilities after deducing them.
  for (spirv::Capability cap : deducedCapabilities) {
    if (std::optional<spirv::Version> minVersion = spirv::getMinVersion(cap)) {
      deducedVersion = std::max(deducedVersion, *minVersion);
      if (deducedVersion > allowedVersion) {
        module.emitError("Capability '")
            << spirv::stringifyCapability(cap) << "' requires min version "
            << spirv::stringifyVersion(deducedVersion)
            << " but target environment allows up to "
            << spirv::stringifyVersion(allowedVersion);
        return signalPassFailure();
      }
    }
  }

  // TODO: verify that the deduced version is consistent with
  // SPIR-V ops' maximal version requirements.

  auto triple = spirv::VerCapExtAttr::get(
      deducedVersion, deducedCapabilities.getArrayRef(),
      deducedExtensions.getArrayRef(), &getContext());
  module->setAttr(spirv::ModuleOp::getVCETripleAttrName(), triple);
}
