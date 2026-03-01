//===- FuncToEmitCPass.cpp - Func to EmitC Pass -----------------*- C++ -*-===//
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
// This file implements a pass to convert the Func dialect to the EmitC dialect.
//
//===----------------------------------------------------------------------===//

#include "mlir/Conversion/FuncToEmitC/FuncToEmitCPass.h"

#include "mlir/Conversion/FuncToEmitC/FuncToEmitC.h"
#include "mlir/Dialect/EmitC/IR/EmitC.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/DialectConversion.h"

namespace mlir {
#define GEN_PASS_DEF_CONVERTFUNCTOEMITC
#include "mlir/Conversion/Passes.h.inc"
} // namespace mlir

using namespace mlir;

namespace {
struct ConvertFuncToEmitC
    : public impl::ConvertFuncToEmitCBase<ConvertFuncToEmitC> {
  void runOnOperation() override;
};
} // namespace

void ConvertFuncToEmitC::runOnOperation() {
  ConversionTarget target(getContext());

  target.addLegalDialect<emitc::EmitCDialect>();
  target.addIllegalOp<func::CallOp, func::FuncOp, func::ReturnOp>();

  RewritePatternSet patterns(&getContext());

  TypeConverter typeConverter;
  typeConverter.addConversion([](Type type) { return type; });

  populateFuncToEmitCPatterns(typeConverter, patterns);

  if (failed(
          applyPartialConversion(getOperation(), target, std::move(patterns))))
    signalPassFailure();
}
