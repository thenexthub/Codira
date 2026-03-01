//===- TosaToMLProgramPass.cpp - Lowering Tosa to MLProgram Dialect--------===//
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
// This transformation pass legalizes the TOSA dialect to the MLProgram dialect.
//
//===----------------------------------------------------------------------===//

#include "mlir/Conversion/TosaToMLProgram/TosaToMLProgram.h"
#include "mlir/Dialect/MLProgram/IR/MLProgram.h"
#include "mlir/Dialect/Tosa/IR/TosaOps.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Pass/PassManager.h"
#include "mlir/Transforms/DialectConversion.h"

namespace mlir {
#define GEN_PASS_DEF_TOSATOMLPROGRAM
#include "mlir/Conversion/Passes.h.inc"
} // namespace mlir

using namespace mlir;
using namespace tosa;

namespace {
struct TosaToMLProgram : public impl::TosaToMLProgramBase<TosaToMLProgram> {
public:
  void runOnOperation() override {
    auto *context = &getContext();
    auto moduleOp = getOperation();

    RewritePatternSet patterns(context);
    ConversionTarget target(*context);
    target.addIllegalOp<tosa::VariableOp, tosa::VariableReadOp,
                        tosa::VariableWriteOp>();
    target.markUnknownOpDynamicallyLegal([](Operation *) { return true; });

    mlir::tosa::populateTosaToMLProgramConversionPatterns(&patterns);

    if (failed(applyPartialConversion(moduleOp, target, std::move(patterns))))
      signalPassFailure();
  }
};
} // namespace
