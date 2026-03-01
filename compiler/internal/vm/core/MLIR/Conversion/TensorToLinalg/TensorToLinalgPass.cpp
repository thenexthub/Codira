//===- TensorToLinalgPass.cpp - Tensor to Linalg Passes -------------------===//
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
// This file implements a pass to convert Tensor dialect to Linalg dialect.
//
//===----------------------------------------------------------------------===//

#include "mlir/Conversion/TensorToLinalg/TensorToLinalgPass.h"

#include "mlir/Conversion/TensorToLinalg/TensorToLinalg.h"
#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Linalg/IR/Linalg.h"
#include "mlir/Dialect/Tensor/IR/Tensor.h"

namespace mlir {
#define GEN_PASS_DEF_CONVERTTENSORTOLINALGPASS
#include "mlir/Conversion/Passes.h.inc"
} // namespace mlir

using namespace mlir;

namespace {
/// A pass converting MLIR Tensor operations into the Linalg dialect.
class ConvertTensorToLinalgPass
    : public impl::ConvertTensorToLinalgPassBase<ConvertTensorToLinalgPass> {
  void runOnOperation() override {
    auto &context = getContext();
    ConversionTarget target(context);
    target
        .addLegalDialect<mlir::arith::ArithDialect, mlir::linalg::LinalgDialect,
                         mlir::tensor::TensorDialect>();
    target.addIllegalOp<mlir::tensor::PadOp>();

    RewritePatternSet patterns(&context);
    populateTensorToLinalgPatterns(patterns);

    if (failed(applyPartialConversion(getOperation(), target,
                                      std::move(patterns))))
      return signalPassFailure();
  }
};
} // namespace
