//===- TosaToMLProgram.cpp - Lowering Tosa to MLProgram Dialect------------===//
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
// These rewriters lower from the TOSA dialect to the MLProgram dialect.
//
//===----------------------------------------------------------------------===//

#include "mlir/Conversion/TosaToMLProgram/TosaToMLProgram.h"
#include "mlir/Dialect/MLProgram/IR/MLProgram.h"
#include "mlir/Dialect/Tosa/IR/TosaOps.h"
#include "mlir/IR/PatternMatch.h"

using namespace mlir;
using namespace tosa;
namespace {

class VariableOpConverter : public OpRewritePattern<tosa::VariableOp> {
public:
  using OpRewritePattern<tosa::VariableOp>::OpRewritePattern;

  LogicalResult matchAndRewrite(tosa::VariableOp op,
                                PatternRewriter &rewriter) const final {
    auto variableType = tosa::getVariableType(op);
    auto newVariable = mlir::ml_program::GlobalOp::create(
        rewriter, op.getLoc(), op.getName(), variableType, /*is_mutable=*/true,
        op.getInitialValueAttr(), /*sym_visibility=*/nullptr);
    newVariable.setPrivate();
    rewriter.replaceOp(op, newVariable);
    return success();
  }
};

class VariableWriteOpConverter
    : public OpRewritePattern<tosa::VariableWriteOp> {
public:
  using OpRewritePattern<tosa::VariableWriteOp>::OpRewritePattern;

  LogicalResult matchAndRewrite(tosa::VariableWriteOp op,
                                PatternRewriter &rewriter) const final {
    auto globalSymbolRef =
        SymbolRefAttr::get(rewriter.getContext(), op.getName());
    auto newVariableWrite = ml_program::GlobalStoreOp::create(
        rewriter, op.getLoc(), globalSymbolRef, op.getInput1());
    rewriter.replaceOp(op, newVariableWrite);
    return success();
  }
};

class VariableReadOpConverter : public OpRewritePattern<tosa::VariableReadOp> {
public:
  using OpRewritePattern<tosa::VariableReadOp>::OpRewritePattern;

  LogicalResult matchAndRewrite(tosa::VariableReadOp op,
                                PatternRewriter &rewriter) const final {
    auto globalSymbolRef =
        SymbolRefAttr::get(rewriter.getContext(), op.getName());
    auto newVariableRead = ml_program::GlobalLoadOp::create(
        rewriter, op.getLoc(), op.getType(), globalSymbolRef);
    rewriter.replaceOp(op, newVariableRead);

    return success();
  }
};

} // namespace

void mlir::tosa::populateTosaToMLProgramConversionPatterns(
    RewritePatternSet *patterns) {
  patterns->add<VariableOpConverter, VariableWriteOpConverter,
                VariableReadOpConverter>(patterns->getContext());
}
