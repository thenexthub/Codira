//===- VectorToSPIRVPass.cpp - Vector to SPIR-V Passes --------------------===//
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
// This file implements a pass to convert Vector dialect to SPIRV dialect.
//
//===----------------------------------------------------------------------===//

#include "mlir/Conversion/VectorToSPIRV/VectorToSPIRVPass.h"

#include "mlir/Conversion/UBToSPIRV/UBToSPIRV.h"
#include "mlir/Conversion/VectorToSPIRV/VectorToSPIRV.h"
#include "mlir/Dialect/SPIRV/IR/SPIRVDialect.h"
#include "mlir/Dialect/SPIRV/Transforms/SPIRVConversion.h"
#include "mlir/Dialect/UB/IR/UBOps.h"
#include "mlir/Transforms/DialectConversion.h"

namespace mlir {
#define GEN_PASS_DEF_CONVERTVECTORTOSPIRVPASS
#include "mlir/Conversion/Passes.h.inc"
} // namespace mlir

using namespace mlir;

namespace {
struct ConvertVectorToSPIRVPass
    : public impl::ConvertVectorToSPIRVPassBase<ConvertVectorToSPIRVPass> {
  void runOnOperation() override;
};
} // namespace

void ConvertVectorToSPIRVPass::runOnOperation() {
  MLIRContext *context = &getContext();
  Operation *op = getOperation();

  auto targetAttr = spirv::lookupTargetEnvOrDefault(op);
  std::unique_ptr<ConversionTarget> target =
      SPIRVConversionTarget::get(targetAttr);

  SPIRVTypeConverter typeConverter(targetAttr);

  // Use UnrealizedConversionCast as the bridge so that we don't need to pull in
  // patterns for other dialects.
  target->addLegalOp<UnrealizedConversionCastOp>();

  RewritePatternSet patterns(context);
  populateVectorToSPIRVPatterns(typeConverter, patterns);
  // Used for folds, e.g. vector.extract[-1] -> ub.poison -> spirv.Undef.
  ub::populateUBToSPIRVConversionPatterns(typeConverter, patterns);

  if (failed(applyPartialConversion(op, *target, std::move(patterns))))
    return signalPassFailure();
}
