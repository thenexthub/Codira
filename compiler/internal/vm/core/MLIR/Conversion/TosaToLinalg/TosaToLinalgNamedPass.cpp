//===- TosaToLinalgPass.cpp - Lowering Tosa to Linalg Dialect -------------===//
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
// This transformation pass legalizes Tosa operations to the Linalg dialect.
//
//===----------------------------------------------------------------------===//

#include "mlir/Conversion/TosaToLinalg/TosaToLinalg.h"

#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/Linalg/IR/Linalg.h"
#include "mlir/Dialect/Math/IR/Math.h"
#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/Dialect/Tensor/IR/Tensor.h"
#include "mlir/Dialect/Tosa/IR/TosaOps.h"
#include "mlir/Dialect/Tosa/Transforms/Passes.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Pass/PassManager.h"
#include "mlir/Transforms/DialectConversion.h"

namespace mlir {
#define GEN_PASS_DEF_TOSATOLINALGNAMED
#include "mlir/Conversion/Passes.h.inc"
} // namespace mlir

using namespace mlir;

namespace {
struct TosaToLinalgNamed
    : public impl::TosaToLinalgNamedBase<TosaToLinalgNamed> {
public:
  TosaToLinalgNamed(const TosaToLinalgNamedOptions &options)
      : impl::TosaToLinalgNamedBase<TosaToLinalgNamed>(options) {}

  void getDependentDialects(DialectRegistry &registry) const override {
    registry
        .insert<arith::ArithDialect, linalg::LinalgDialect, math::MathDialect,
                tensor::TensorDialect, scf::SCFDialect>();
  }

  void runOnOperation() override {
    TypeConverter converter;
    tosa::populateTosaTypeConversion(converter);

    RewritePatternSet patterns(&getContext());
    ConversionTarget target(getContext());
    target.addLegalDialect<linalg::LinalgDialect, tosa::TosaDialect,
                           tensor::TensorDialect, scf::SCFDialect>();

    // Not every TOSA op can be legalized to linalg.
    target.addIllegalOp<tosa::Conv2DOp>();
    target.addIllegalOp<tosa::Conv3DOp>();
    target.addIllegalOp<tosa::DepthwiseConv2DOp>();
    target.addIllegalOp<tosa::MaxPool2dOp>();
    target.addIllegalOp<tosa::AvgPool2dOp>();
    target.addIllegalOp<tosa::MatMulOp>();
    target.addIllegalOp<tosa::TransposeOp>();

    target.markUnknownOpDynamicallyLegal([](Operation *) { return true; });

    FunctionOpInterface func = getOperation();
    TosaToLinalgNamedOptions options;
    options.preferConv2DKernelLayoutHWCF = preferConv2DKernelLayoutHWCF;
    tosa::populateTosaToLinalgNamedConversionPatterns(converter, &patterns,
                                                      options);
    if (failed(applyFullConversion(func, target, std::move(patterns))))
      signalPassFailure();
  }
};
} // namespace

std::unique_ptr<Pass>
mlir::tosa::createTosaToLinalgNamed(const TosaToLinalgNamedOptions &options) {
  return std::make_unique<TosaToLinalgNamed>(options);
}
