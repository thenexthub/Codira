//===- TosaToTensorPass.cpp - Lowering Tosa to Tensor Dialect -------------===//
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
// This transformation pass legalizes Tosa operations to the Tensor dialect.
//
//===----------------------------------------------------------------------===//

#include "mlir/Conversion/TosaToTensor/TosaToTensor.h"

#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Tensor/IR/Tensor.h"
#include "mlir/Dialect/Tosa/IR/TosaOps.h"
#include "mlir/Dialect/Tosa/Transforms/Passes.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Transforms/DialectConversion.h"

namespace mlir {
#define GEN_PASS_DEF_TOSATOTENSORPASS
#include "mlir/Conversion/Passes.h.inc"
} // namespace mlir

using namespace mlir;
using namespace tosa;

namespace {
struct TosaToTensor : public impl::TosaToTensorPassBase<TosaToTensor> {
public:
  void runOnOperation() override {
    RewritePatternSet patterns(&getContext());
    ConversionTarget target(getContext());
    target.addIllegalOp<tosa::ConcatOp>();
    target.addIllegalOp<tosa::ReshapeOp>();
    target.addIllegalOp<tosa::SliceOp>();
    target.addIllegalOp<tosa::PadOp>();
    target.addLegalDialect<arith::ArithDialect>();
    target.addLegalDialect<tensor::TensorDialect>();

    TypeConverter converter;
    mlir::tosa::populateTosaTypeConversion(converter);

    mlir::tosa::populateTosaToTensorConversionPatterns(converter, &patterns);

    if (failed(applyPartialConversion(getOperation(), target,
                                      std::move(patterns))))
      signalPassFailure();
  }
};
} // namespace
