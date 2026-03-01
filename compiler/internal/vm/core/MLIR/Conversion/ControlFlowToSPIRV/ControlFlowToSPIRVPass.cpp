//===- ControlFlowToSPIRVPass.cpp - ControlFlow to SPIR-V Pass ------------===//
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
// This file implements a pass to convert ControlFlow dialect to SPIR-V dialect.
//
//===----------------------------------------------------------------------===//

#include "mlir/Conversion/ControlFlowToSPIRV/ControlFlowToSPIRVPass.h"

#include "mlir/Conversion/ControlFlowToSPIRV/ControlFlowToSPIRV.h"
#include "mlir/Dialect/SPIRV/IR/SPIRVDialect.h"
#include "mlir/Dialect/SPIRV/Transforms/SPIRVConversion.h"

namespace mlir {
#define GEN_PASS_DEF_CONVERTCONTROLFLOWTOSPIRVPASS
#include "mlir/Conversion/Passes.h.inc"
} // namespace mlir

using namespace mlir;

namespace {
/// A pass converting MLIR ControlFlow operations into the SPIR-V dialect.
class ConvertControlFlowToSPIRVPass final
    : public impl::ConvertControlFlowToSPIRVPassBase<
          ConvertControlFlowToSPIRVPass> {
  using Base::Base;
  void runOnOperation() override;
};
} // namespace

void ConvertControlFlowToSPIRVPass::runOnOperation() {
  MLIRContext *context = &getContext();
  Operation *op = getOperation();

  auto targetAttr = spirv::lookupTargetEnvOrDefault(op);
  std::unique_ptr<ConversionTarget> target =
      SPIRVConversionTarget::get(targetAttr);

  SPIRVConversionOptions options;
  options.emulateLT32BitScalarTypes = this->emulateLT32BitScalarTypes;
  options.emulateUnsupportedFloatTypes = this->emulateUnsupportedFloatTypes;
  SPIRVTypeConverter typeConverter(targetAttr, options);

  // TODO: We should also take care of block argument type conversion.

  RewritePatternSet patterns(context);
  cf::populateControlFlowToSPIRVPatterns(typeConverter, patterns);

  if (failed(applyPartialConversion(op, *target, std::move(patterns))))
    return signalPassFailure();
}
