//===- NamedOpConversions.cpp - Implements conversions between named ops --===//
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
// This file implements conversions between named ops that can be seens as
// canonicalizations of named ops.
//
//===----------------------------------------------------------------------===//

#include "mlir/Dialect/Linalg/Passes.h"

#include "mlir/Dialect/Linalg/IR/Linalg.h"
#include "mlir/Dialect/Linalg/Transforms/Transforms.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"
#include "vm/core/ADT/SmallVector.h"
#include "vm/core/ADT/TypeSwitch.h"

namespace mlir {
#define GEN_PASS_DEF_SIMPLIFYDEPTHWISECONVPASS
#include "mlir/Dialect/Linalg/Passes.h.inc"
} // namespace mlir

using namespace mlir;
using namespace mlir::linalg;

static toolchain::SmallVector<int64_t> getIndicesVector(int start, int end) {
  return toolchain::to_vector<2>(toolchain::seq<int64_t>(start, end));
}

static LogicalResult
matchAndReplaceDepthwiseConv(Operation *operation, Value input, Value kernel,
                             Value iZp, Value kZp, Value init, Attribute stride,
                             Attribute dilation, PatternRewriter &rewriter) {
  Location loc = operation->getLoc();
  auto linalgOp = dyn_cast<LinalgOp>(operation);
  // Exit out on the memref version of this operation.
  if (!linalgOp || !linalgOp.hasPureTensorSemantics())
    return failure();

  auto result = operation->getResult(0);

  auto kernelTy = dyn_cast<RankedTensorType>(kernel.getType());
  auto initTy = dyn_cast<RankedTensorType>(init.getType());
  auto resultTy = dyn_cast<RankedTensorType>(result.getType());
  if (!kernelTy || !initTy || !resultTy)
    return failure();

  if (kernelTy.getDimSize(3) != 1)
    return failure();

  // Collapse kernel dims.
  SmallVector<ReassociationIndices, 4> collapsedKernelDims = {
      getIndicesVector(0, 1), getIndicesVector(1, 2), getIndicesVector(2, 4)};
  auto newKernelTy = RankedTensorType::get(
      {kernelTy.getDimSize(0), kernelTy.getDimSize(1), kernelTy.getDimSize(2)},
      kernelTy.getElementType());
  auto collapsedKernel = tensor::CollapseShapeOp::create(
      rewriter, loc, newKernelTy, kernel, collapsedKernelDims);

  // Collapse init dims.
  SmallVector<ReassociationIndices, 4> collapsedInitDims = {
      getIndicesVector(0, 1), getIndicesVector(1, 2), getIndicesVector(2, 3),
      getIndicesVector(3, 5)};
  auto newInitTy =
      RankedTensorType::get({initTy.getDimSize(0), initTy.getDimSize(1),
                             initTy.getDimSize(2), initTy.getDimSize(3)},
                            initTy.getElementType());
  auto collapsedInit = tensor::CollapseShapeOp::create(rewriter, loc, newInitTy,
                                                       init, collapsedInitDims);

  SmallVector<NamedAttribute> preservedAttrs;
  Operation *newConv =
      TypeSwitch<Operation *, Operation *>(operation)
          .Case<DepthwiseConv2DNhwcHwcmOp>([&](auto op) {
            preservedAttrs = getPrunedAttributeList(op);
            return DepthwiseConv2DNhwcHwcOp::create(
                rewriter, loc, newInitTy, ValueRange{input, collapsedKernel},
                ValueRange{collapsedInit}, stride, dilation);
          })
          .Case<DepthwiseConv2DNhwcHwcmQOp>([&](auto op) {
            preservedAttrs = getPrunedAttributeList(op);
            return DepthwiseConv2DNhwcHwcQOp::create(
                rewriter, loc, newInitTy,
                ValueRange{input, collapsedKernel, iZp, kZp},
                ValueRange{collapsedInit}, stride, dilation);
          })
          .Default(nullptr);
  if (!newConv)
    return failure();
  for (auto attr : preservedAttrs)
    newConv->setAttr(attr.getName(), attr.getValue());

  // Expand dimensions back out to
  rewriter.replaceOpWithNewOp<tensor::ExpandShapeOp>(
      operation, resultTy, newConv->getResult(0), collapsedInitDims);
  return success();
}

namespace {
struct SimplifyDepthwiseConvOp
    : public OpRewritePattern<DepthwiseConv2DNhwcHwcmOp> {
  using OpRewritePattern<DepthwiseConv2DNhwcHwcmOp>::OpRewritePattern;

  LogicalResult matchAndRewrite(DepthwiseConv2DNhwcHwcmOp op,
                                PatternRewriter &rewriter) const override {
    Operation *operation = op.getOperation();
    Value input = op.getDpsInputOperand(0)->get();
    Value kernel = op.getDpsInputOperand(1)->get();
    Value init = op.getDpsInitOperand(0)->get();

    auto stride = op.getStrides();
    auto dilation = op.getDilations();

    return matchAndReplaceDepthwiseConv(operation, input, kernel, nullptr,
                                        nullptr, init, stride, dilation,
                                        rewriter);
  }
};

struct SimplifyDepthwiseConvQOp
    : public OpRewritePattern<DepthwiseConv2DNhwcHwcmQOp> {
  using OpRewritePattern<DepthwiseConv2DNhwcHwcmQOp>::OpRewritePattern;

  LogicalResult matchAndRewrite(DepthwiseConv2DNhwcHwcmQOp op,
                                PatternRewriter &rewriter) const override {
    Operation *operation = op.getOperation();
    Value input = op.getDpsInputOperand(0)->get();
    Value kernel = op.getDpsInputOperand(1)->get();
    Value iZp = op.getDpsInputOperand(2)->get();
    Value kZp = op.getDpsInputOperand(3)->get();
    Value init = op.getDpsInitOperand(0)->get();

    auto stride = op.getStrides();
    auto dilation = op.getDilations();

    return matchAndReplaceDepthwiseConv(operation, input, kernel, iZp, kZp,
                                        init, stride, dilation, rewriter);
  }
};

struct SimplifyDepthwiseConvPass
    : public impl::SimplifyDepthwiseConvPassBase<SimplifyDepthwiseConvPass> {
  using impl::SimplifyDepthwiseConvPassBase<
      SimplifyDepthwiseConvPass>::SimplifyDepthwiseConvPassBase;

  void runOnOperation() override {
    Operation *op = getOperation();
    RewritePatternSet patterns(op->getContext());
    populateSimplifyDepthwiseConvPatterns(patterns);
    if (failed(applyPatternsGreedily(op, std::move(patterns))))
      return signalPassFailure();
  }
};
} // namespace

void mlir::linalg::populateSimplifyDepthwiseConvPatterns(
    RewritePatternSet &patterns) {
  patterns.add<SimplifyDepthwiseConvOp, SimplifyDepthwiseConvQOp>(
      patterns.getContext());
}
