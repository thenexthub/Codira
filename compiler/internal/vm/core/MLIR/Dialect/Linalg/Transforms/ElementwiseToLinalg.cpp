//===- ElementwiseToLinalg.cpp - conversion of elementwise to linalg ------===//
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

#include "mlir/Dialect/Linalg/Passes.h"

#include "mlir/Dialect/Linalg/IR/Linalg.h"
#include "mlir/Dialect/Linalg/Transforms/Transforms.h"
#include "mlir/Dialect/Linalg/Utils/Utils.h"
#include "mlir/Transforms/DialectConversion.h"

namespace mlir {
#define GEN_PASS_DEF_CONVERTELEMENTWISETOLINALGPASS
#include "mlir/Dialect/Linalg/Passes.h.inc"
} // namespace mlir

using namespace mlir;

static inline bool isScalarLike(Type t) {
  return isa<IntegerType, FloatType, IndexType, ComplexType>(t);
}

static bool isElementwiseMappableOpOnRankedTensors(Operation *op) {
  if (!OpTrait::hasElementwiseMappableTraits(op))
    return false;

  auto types = op->getOperandTypes();

  // We want at least one ranked tensor.
  bool anyRankedTensor = toolchain::any_of(types, toolchain::IsaPred<RankedTensorType>);

  // No invalid operands (i.e., every operand is a ranked tensor or
  // scalar-like).
  bool noneInvalid = toolchain::none_of(types, [](Type t) {
    return !(isa<RankedTensorType>(t) || isScalarLike(t));
  });

  return anyRankedTensor && noneInvalid;
}

/// Given `op` assumed `isElementwiseMappableOpOnRankedTensors`, iterate over
/// the result types and return a list of values such that, for each result type
/// `t` and value `v` at the same index `idx`:
///   1. `v.getType() == t`
///   2. If an operand of `op` has type `t`, let `operand_first` be the first
///      such operand. Then`v == operand_first`.
///   3. Otherwise, v is a newly created `tensor::EmptyOp` with:
///        a. Static and dynamic dims extracted from the first operand of `op`.
///        b. Elemental type equal to the elemental type of `t`.
///
/// This is sufficient because ElementwiseMappable guarantees that "The static
/// types of all vector (resp. tensor) operands and results must have the same
/// shape".
static SmallVector<Value, 4>
getOrCreateOperandsMatchingResultTypes(OpBuilder &b, Operation *op) {
  assert(isElementwiseMappableOpOnRankedTensors(op));
  Location loc = op->getLoc();
  ValueRange operands = op->getOperands();
  TypeRange rankedTensorTypes = op->getResultTypes();
  SmallVector<Value, 4> res;
  res.reserve(rankedTensorTypes.size());
  for (Type t : rankedTensorTypes) {
    // Try to find an operand with type matching the result tensor.
    bool found = false;
    for (Value v : operands) {
      if (v.getType() == t) {
        found = true;
        res.push_back(v);
        break;
      }
    }
    if (found)
      continue;

    // Extract static / dynamic shape mix from the first operand.
    res.push_back(tensor::EmptyOp::create(
        b, loc, tensor::getMixedSizes(b, loc, operands.front()),
        cast<RankedTensorType>(t).getElementType()));
  }
  return res;
}

namespace {
struct ConvertAnyElementwiseMappableOpOnRankedTensors : public RewritePattern {
  ConvertAnyElementwiseMappableOpOnRankedTensors(MLIRContext *context)
      : RewritePattern(MatchAnyOpTypeTag(), /*benefit=*/1, context) {}
  LogicalResult matchAndRewrite(Operation *op,
                                PatternRewriter &rewriter) const final {
    if (!isElementwiseMappableOpOnRankedTensors(op))
      return rewriter.notifyMatchFailure(
          op, "requires elementwise op on ranked tensors");

    auto resTy = cast<RankedTensorType>(op->getResult(0).getType());
    auto rank = resTy.getRank();

    // Maps: identity for tensors (rank > 0), scalar map for scalars.
    AffineMap scalarMap = AffineMap::get(/*dimCount=*/rank, /*symbolCount=*/0,
                                         /*results=*/{}, rewriter.getContext());
    AffineMap idMap = rewriter.getMultiDimIdentityMap(rank);

    // Match phase.
    SmallVector<bool> isScalarOperand;
    isScalarOperand.reserve(op->getNumOperands());
    for (Type ty : op->getOperandTypes()) {
      if (isScalarLike(ty))
        isScalarOperand.push_back(true);
      else if (auto rt = dyn_cast<RankedTensorType>(ty))
        isScalarOperand.push_back(false);
      else
        return rewriter.notifyMatchFailure(
            op,
            "unsupported operand type (expected scalar-like or ranked tensor)");
    }

    // Create indexing maps.
    SmallVector<AffineMap> indexingMaps;
    indexingMaps.reserve(op->getNumOperands() + op->getNumResults());

    for (bool isScalar : isScalarOperand)
      indexingMaps.push_back(isScalar ? scalarMap : idMap);

    indexingMaps.append(op->getNumResults(), idMap);

    SmallVector<utils::IteratorType> iteratorTypes(
        rank, utils::IteratorType::parallel);
    SmallVector<Value> outputs =
        getOrCreateOperandsMatchingResultTypes(rewriter, op);
    rewriter.replaceOpWithNewOp<linalg::GenericOp>(
        op, /*resultTensorTypes=*/op->getResultTypes(),
        /*inputs=*/op->getOperands(),
        /*outputs=*/outputs,
        /*indexingMaps=*/indexingMaps,
        /*iteratorTypes=*/iteratorTypes,
        /*bodyBuilder=*/
        [&](OpBuilder &builder, Location loc, ValueRange regionArgs) {
          SmallVector<Type> resultEltTys = toolchain::to_vector<6>(
              toolchain::map_range(op->getResultTypes(), [](Type type) {
                return cast<TensorType>(type).getElementType();
              }));
          Operation *scalarOp =
              builder.create(loc, op->getName().getIdentifier(),
                             regionArgs.take_front(op->getNumOperands()),
                             resultEltTys, op->getAttrs());
          linalg::YieldOp::create(builder, loc, scalarOp->getResults());
        });
    return success();
  }
};
} // namespace

void mlir::linalg::populateElementwiseToLinalgConversionPatterns(
    RewritePatternSet &patterns) {
  patterns.add<ConvertAnyElementwiseMappableOpOnRankedTensors>(
      patterns.getContext());
}

namespace {
class ConvertElementwiseToLinalgPass
    : public impl::ConvertElementwiseToLinalgPassBase<
          ConvertElementwiseToLinalgPass> {
  using impl::ConvertElementwiseToLinalgPassBase<
      ConvertElementwiseToLinalgPass>::ConvertElementwiseToLinalgPassBase;

  void runOnOperation() final {
    auto *func = getOperation();
    auto *context = &getContext();
    ConversionTarget target(*context);
    RewritePatternSet patterns(context);

    mlir::linalg::populateElementwiseToLinalgConversionPatterns(patterns);
    target.markUnknownOpDynamicallyLegal([](Operation *op) {
      return !isElementwiseMappableOpOnRankedTensors(op);
    });

    if (failed(applyPartialConversion(func, target, std::move(patterns))))
      signalPassFailure();
  }
};
} // namespace
