//===- SCFToSPIRVPass.cpp - SCF to SPIR-V Passes --------------------------===//
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
// This file implements a pass to convert SCF dialect into SPIR-V dialect.
//
//===----------------------------------------------------------------------===//

#include "mlir/Conversion/SCFToSPIRV/SCFToSPIRVPass.h"

#include "mlir/Conversion/ArithToSPIRV/ArithToSPIRV.h"
#include "mlir/Conversion/FuncToSPIRV/FuncToSPIRV.h"
#include "mlir/Conversion/IndexToSPIRV/IndexToSPIRV.h"
#include "mlir/Conversion/MemRefToSPIRV/MemRefToSPIRV.h"
#include "mlir/Conversion/SCFToSPIRV/SCFToSPIRV.h"
#include "mlir/Dialect/SPIRV/IR/SPIRVDialect.h"
#include "mlir/Dialect/SPIRV/Transforms/SPIRVConversion.h"
#include "mlir/IR/BuiltinOps.h"

namespace mlir {
#define GEN_PASS_DEF_SCFTOSPIRV
#include "mlir/Conversion/Passes.h.inc"
} // namespace mlir

using namespace mlir;

namespace {
struct SCFToSPIRVPass : public impl::SCFToSPIRVBase<SCFToSPIRVPass> {
  void runOnOperation() override;
};
} // namespace

void SCFToSPIRVPass::runOnOperation() {
  MLIRContext *context = &getContext();
  Operation *op = getOperation();

  auto targetAttr = spirv::lookupTargetEnvOrDefault(op);
  std::unique_ptr<ConversionTarget> target =
      SPIRVConversionTarget::get(targetAttr);
  target->addLegalOp<UnrealizedConversionCastOp>();

  SPIRVTypeConverter typeConverter(targetAttr);
  ScfToSPIRVContext scfContext;
  RewritePatternSet patterns(context);
  populateSCFToSPIRVPatterns(typeConverter, scfContext, patterns);

  // TODO: Change SPIR-V conversion to be progressive and remove the following
  // patterns.
  mlir::arith::populateArithToSPIRVPatterns(typeConverter, patterns);
  populateFuncToSPIRVPatterns(typeConverter, patterns);
  populateMemRefToSPIRVPatterns(typeConverter, patterns);
  populateBuiltinFuncToSPIRVPatterns(typeConverter, patterns);
  index::populateIndexToSPIRVPatterns(typeConverter, patterns);

  if (failed(applyPartialConversion(op, *target, std::move(patterns))))
    return signalPassFailure();
}
