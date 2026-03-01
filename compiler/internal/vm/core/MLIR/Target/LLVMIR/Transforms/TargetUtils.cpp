//===- TargetUtils.cpp - utils for obtaining generic target backend info --===//
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

#include "mlir/Dialect/DLTI/DLTI.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/Target/LLVMIR/Import.h"

#include "vm/core/MC/TargetRegistry.h"
#include "vm/core/Support/DebugLog.h"
#include "vm/core/Support/TargetSelect.h"
#include "vm/core/Target/TargetMachine.h"

#define DEBUG_TYPE "mlir-toolchain-target-utils"

namespace mlir {
namespace LLVM {
namespace detail {
void initializeBackendsOnce() {
  static const auto initOnce = [] {
    // Ensure that the targets, that LLVM has been configured to support,
    // are loaded into the TargetRegistry.
    toolchain::InitializeAllTargets();
    toolchain::InitializeAllTargetMCs();
    return true;
  }();
  (void)initOnce; // Dummy usage.
}

FailureOr<std::unique_ptr<toolchain::TargetMachine>>
getTargetMachine(mlir::LLVM::TargetAttrInterface attr) {
  StringRef triple = attr.getTriple();
  StringRef chipAKAcpu = attr.getChip();
  // NB: `TargetAttrInterface::getFeatures()` is coarsely typed to work around
  // cyclic dependency issue in tablegen files.
  auto featuresAttr =
      toolchain::cast_if_present<LLVM::TargetFeaturesAttr>(attr.getFeatures());
  std::string features = featuresAttr ? featuresAttr.getFeaturesString() : "";

  toolchain::Triple parsedTriple(triple);
  std::string error;
  const toolchain::Target *target =
      toolchain::TargetRegistry::lookupTarget(parsedTriple, error);
  if (!target || !error.empty()) {
    LDBG() << "Looking up target '" << triple << "' failed: " << error << "\n";
    return failure();
  }

  return std::unique_ptr<toolchain::TargetMachine>(
      target->createTargetMachine(parsedTriple, chipAKAcpu, features, {}, {}));
}

FailureOr<toolchain::DataLayout>
getDataLayout(mlir::LLVM::TargetAttrInterface attr) {
  FailureOr<std::unique_ptr<toolchain::TargetMachine>> targetMachine =
      getTargetMachine(attr);
  if (failed(targetMachine)) {
    LDBG() << "Failed to retrieve the target machine for data layout.\n";
    return failure();
  }
  return (targetMachine.value())->createDataLayout();
}

} // namespace detail
} // namespace LLVM
} // namespace mlir
