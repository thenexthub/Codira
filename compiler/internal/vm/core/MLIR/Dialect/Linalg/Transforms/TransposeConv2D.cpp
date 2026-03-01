//===- TransposeConv2D.cpp - Convolution transposition  -------------------===//
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

#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/Linalg/IR/Linalg.h"
#include "mlir/Dialect/MemRef/IR/MemRef.h"
#include "mlir/Dialect/Tensor/IR/Tensor.h"
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Transforms/DialectConversion.h"
#include "vm/core/ADT/SmallVector.h"

namespace mlir {
namespace linalg {
namespace {
// clang-format off
/// Convolution converter that applies the following rewrite:
///
/// Before:
///
///   %0 = linalg.conv_2d_nhwc_fhwc {dilations = dense<1> : tensor<2xi64>,
///                                               strides = dense<2> : tensor<2xi64>}
///      ins (%input, %filter: tensor<1x4x4x6xf32>, tensor<8x2x2x6xf32>)
///     outs (%init: tensor<1x2x2x8xf32>) -> tensor<1x2x2x8xf32>
///
/// After:
///
///    %cst = arith.constant 0.000000e+00 : f32
///    %0 = tensor.empty() : tensor<2x2x6x8xf32>
///    %1 = linalg.fill ins(%cst : f32) outs(%0 : tensor<2x2x6x8xf32>) -> tensor<2x2x6x8xf32>
///    %transposed = linalg.transpose ins(%arg1 : tensor<8x2x2x6xf32>) outs(%1 : tensor<2x2x6x8xf32>)
///                  permutation = [1, 2, 3, 0]
///    %2 = linalg.conv_2d_nhwc_hwcf {dilations = dense<1> : tensor<2xi64>, strides = dense<2> : tensor<2xi64>}
///         ins(%arg0, %transposed : tensor<1x4x4x6xf32>, tensor<2x2x6x8xf32>) outs(%arg2 : tensor<1x2x2x8xf32>)
///         -> tensor<1x2x2x8xf32>
///
/// with an analogous example for the quantized case.
// clang-format on
template <typename FHWCConvOp, typename HWCFConvOp>
FailureOr<Operation *> transposeConv2DHelper(RewriterBase &rewriter,
                                             FHWCConvOp op) {
  // Construct a permutation of the filter tensor dimensions. For a 2D
  // convolution this will be known statically as [1, 2, 3, 0].
  SmallVector<int64_t> filterPerm = {1, 2, 3, 0};

  // Create the type for the transposed filter tensor.
  auto filter = op->getOperand(1);
  auto filterTy = cast<ShapedType>(filter.getType());
  SmallVector<int64_t> newFilterShape(filterPerm.size());
  std::generate(std::begin(newFilterShape), std::end(newFilterShape),
                [dim = 0, &filterTy, &filterPerm]() mutable {
                  return filterTy.getShape()[filterPerm[dim++]];
                });

  // Because linalg.transpose expects an "out" parameter we need to pass it a
  // tensor of zeros of the result type so here we construct that tensor.
  auto inputType = op->getOperand(0).getType();
  auto elementTy = cast<ShapedType>(inputType).getElementType();
  auto loc = op->getLoc();

  const auto isTensorOp = isa<TensorType>(inputType);
  Value input;
  if (isTensorOp) {

    input = tensor::EmptyOp::create(rewriter, loc, newFilterShape, elementTy)
                .getResult();
  } else {
    input = memref::AllocOp::create(rewriter, loc,
                                    MemRefType::get(newFilterShape, elementTy))
                .getResult();
  }

  // We can then construct the transposition on our filter.
  auto transpose =
      linalg::TransposeOp::create(rewriter, loc, filter, input, filterPerm);

  Value newFilter;
  if (isTensorOp) {
    newFilter = transpose.getResult()[0];
  } else {
    newFilter = input;
  }

  SmallVector<Value> newInputs{op.getInputs()};
  // The filter is always the second input argument, the other inputs can be
  // left as they are.
  newInputs[1] = newFilter;
  // It is possible the convolution doesn't define any results and its
  // out argument is just used instead.
  SmallVector<Type> resultTy;
  if (op.getNumResults()) {
    resultTy.push_back(op->getResult(0).getType());
  }
  auto newConv =
      HWCFConvOp::create(rewriter, loc, resultTy, newInputs, op.getOutputs(),
                         op.getStrides(), op.getDilations());
  rewriter.replaceOp(op, newConv);
  return newConv.getOperation();
}

template <typename FHWCConvOp, typename HWCFConvOp>
class ConvConverter : public OpRewritePattern<FHWCConvOp> {
public:
  using OpRewritePattern<FHWCConvOp>::OpRewritePattern;
  LogicalResult matchAndRewrite(FHWCConvOp op,
                                PatternRewriter &rewriter) const final {
    if (failed(transposeConv2DHelper<FHWCConvOp, HWCFConvOp>(rewriter, op))) {
      return failure();
    }
    return success();
  }
};
} // namespace

FailureOr<Operation *> transposeConv2D(RewriterBase &rewriter,
                                       linalg::Conv2DNhwcFhwcOp op) {

  return transposeConv2DHelper<linalg::Conv2DNhwcFhwcOp,
                               linalg::Conv2DNhwcHwcfOp>(rewriter, op);
}

FailureOr<Operation *> transposeConv2D(RewriterBase &rewriter,
                                       linalg::Conv2DNhwcFhwcQOp op) {

  return transposeConv2DHelper<linalg::Conv2DNhwcFhwcQOp,
                               linalg::Conv2DNhwcHwcfQOp>(rewriter, op);
}

void populateTransposeConv2DPatterns(RewritePatternSet &patterns) {
  MLIRContext *context = patterns.getContext();
  patterns.insert<
      ConvConverter<linalg::Conv2DNhwcFhwcOp, linalg::Conv2DNhwcHwcfOp>,
      ConvConverter<linalg::Conv2DNhwcFhwcQOp, linalg::Conv2DNhwcHwcfQOp>>(
      context);
}
} // namespace linalg
} // namespace mlir
