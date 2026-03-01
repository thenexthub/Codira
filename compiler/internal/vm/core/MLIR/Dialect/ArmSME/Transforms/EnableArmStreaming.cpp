//===- EnableArmStreaming.cpp - Enable Armv9 Streaming SVE mode -----------===//
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
// This pass enables the Armv9 Scalable Matrix Extension (SME) Streaming SVE
// (SSVE) mode [1][2] by adding either of the following attributes to
// 'func.func' ops:
//
//   * 'arm_streaming' (default)
//   * 'arm_locally_streaming'
//
// It can also optionally enable the ZA storage array.
//
// Streaming-mode is part of the interface (ABI) for functions with the
// first attribute and it's the responsibility of the caller to manage
// PSTATE.SM on entry/exit to functions with this attribute [3]. The LLVM
// backend will emit 'smstart sm' / 'smstop sm' [4] around calls to
// streaming functions.
//
// In locally streaming functions PSTATE.SM is kept internal and managed by
// the callee on entry/exit. The LLVM backend will emit 'smstart sm' /
// 'smstop sm' in the prologue / epilogue for functions with this
// attribute.
//
// [1] https://developer.arm.com/documentation/ddi0616/aa
// [2] https://toolchain.org/docs/AArch64SME.html
// [3] https://github.com/ARM-software/abi-aa/blob/main/aapcs64/aapcs64.rst#671pstatesm-interfaces
// [4] https://developer.arm.com/documentation/ddi0602/2023-03/Base-Instructions/SMSTART--Enables-access-to-Streaming-SVE-mode-and-SME-architectural-state--an-alias-of-MSR--immediate--
//
//===----------------------------------------------------------------------===//

#include "mlir/Dialect/ArmSME/IR/ArmSME.h"
#include "mlir/Dialect/ArmSME/Transforms/Passes.h"
#include "mlir/Dialect/ArmSME/Transforms/PassesEnums.cpp.inc"

#include "mlir/Dialect/Func/IR/FuncOps.h"

#define DEBUG_TYPE "enable-arm-streaming"

namespace mlir {
namespace arm_sme {
#define GEN_PASS_DEF_ENABLEARMSTREAMING
#include "mlir/Dialect/ArmSME/Transforms/Passes.h.inc"
} // namespace arm_sme
} // namespace mlir

using namespace mlir;
using namespace mlir::arm_sme;
namespace {

constexpr StringLiteral
    kEnableArmStreamingIgnoreAttr("enable_arm_streaming_ignore");

template <typename... Ops>
constexpr auto opList() {
  return std::array{TypeID::get<Ops>()...};
}

bool isScalableVector(Type type) {
  if (auto vectorType = dyn_cast<VectorType>(type))
    return vectorType.isScalable();
  return false;
}

struct EnableArmStreamingPass
    : public arm_sme::impl::EnableArmStreamingBase<EnableArmStreamingPass> {
  EnableArmStreamingPass(ArmStreamingMode streamingMode, ArmZaMode zaMode,
                         bool ifRequiredByOps, bool ifScalableAndSupported) {
    this->streamingMode = streamingMode;
    this->zaMode = zaMode;
    this->ifRequiredByOps = ifRequiredByOps;
    this->ifScalableAndSupported = ifScalableAndSupported;
  }
  void runOnOperation() override {
    auto function = getOperation();

    if (ifRequiredByOps && ifScalableAndSupported) {
      function->emitOpError(
          "enable-arm-streaming: `if-required-by-ops` and "
          "`if-scalable-and-supported` are mutually exclusive");
      return signalPassFailure();
    }

    if (ifRequiredByOps) {
      bool foundTileOp = false;
      function.walk([&](Operation *op) {
        if (toolchain::isa<ArmSMETileOpInterface>(op)) {
          foundTileOp = true;
          return WalkResult::interrupt();
        }
        return WalkResult::advance();
      });
      if (!foundTileOp)
        return;
    }

    if (ifScalableAndSupported) {
      // FIXME: This should be based on target information (i.e., the presence
      // of FEAT_SME_FA64). This currently errs on the side of caution. If
      // possible gathers/scatters should be lowered regular vector loads/stores
      // before invoking this pass.
      auto disallowedOperations = opList<vector::GatherOp, vector::ScatterOp>();
      bool isCompatibleScalableFunction = false;
      function.walk([&](Operation *op) {
        if (toolchain::is_contained(disallowedOperations,
                               op->getName().getTypeID())) {
          isCompatibleScalableFunction = false;
          return WalkResult::interrupt();
        }
        if (!isCompatibleScalableFunction &&
            (toolchain::any_of(op->getOperandTypes(), isScalableVector) ||
             toolchain::any_of(op->getResultTypes(), isScalableVector))) {
          isCompatibleScalableFunction = true;
        }
        return WalkResult::advance();
      });
      if (!isCompatibleScalableFunction)
        return;
    }

    if (function->getAttr(kEnableArmStreamingIgnoreAttr) ||
        streamingMode == ArmStreamingMode::Disabled)
      return;

    auto unitAttr = UnitAttr::get(&getContext());

    function->setAttr(stringifyArmStreamingMode(streamingMode), unitAttr);

    // The pass currently only supports enabling ZA when in streaming-mode, but
    // ZA can be accessed by the SME LDR, STR and ZERO instructions when not in
    // streaming-mode (see section B1.1.1, IDGNQM of spec [1]). It may be worth
    // supporting this later.
    if (zaMode != ArmZaMode::Disabled)
      function->setAttr(stringifyArmZaMode(zaMode), unitAttr);
  }
};
} // namespace

std::unique_ptr<Pass> mlir::arm_sme::createEnableArmStreamingPass(
    const ArmStreamingMode streamingMode, const ArmZaMode zaMode,
    bool ifRequiredByOps, bool ifScalableAndSupported) {
  return std::make_unique<EnableArmStreamingPass>(
      streamingMode, zaMode, ifRequiredByOps, ifScalableAndSupported);
}
