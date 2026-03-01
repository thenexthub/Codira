//===- RuntimeOpVerification.cpp - Op Verification ------------------------===//
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

#include "mlir/Dialect/Linalg/Transforms/RuntimeOpVerification.h"

#include "mlir/Dialect/Affine/IR/AffineOps.h"
#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Arith/Utils/Utils.h"
#include "mlir/Dialect/ControlFlow/IR/ControlFlowOps.h"
#include "mlir/Dialect/Index/IR/IndexAttrs.h"
#include "mlir/Dialect/Index/IR/IndexDialect.h"
#include "mlir/Dialect/Index/IR/IndexOps.h"
#include "mlir/Dialect/Linalg/IR/Linalg.h"
#include "mlir/Dialect/MemRef/IR/MemRef.h"
#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/Dialect/Tensor/IR/Tensor.h"
#include "mlir/Interfaces/RuntimeVerifiableOpInterface.h"

namespace mlir {
namespace linalg {
namespace {
/// Verify that the runtime sizes of the operands to linalg structured ops are
/// compatible with the runtime sizes inferred by composing the loop ranges with
/// the linalg op's indexing maps. This is similar to the verifier except that
/// here we insert IR to perform the verification at runtime.
template <typename T>
struct StructuredOpInterface
    : public RuntimeVerifiableOpInterface::ExternalModel<
          StructuredOpInterface<T>, T> {
  void
  generateRuntimeVerification(Operation *op, OpBuilder &builder, Location loc,
                              function_ref<std::string(Operation *, StringRef)>
                                  generateErrorMessage) const {
    auto linalgOp = toolchain::cast<LinalgOp>(op);

    SmallVector<Range> loopRanges = linalgOp.createLoopRanges(builder, loc);
    auto [starts, ends, _] = getOffsetsSizesAndStrides(loopRanges);

    auto zero = arith::ConstantIndexOp::create(builder, loc, 0);
    auto one = arith::ConstantIndexOp::create(builder, loc, 1);

    Value iterationDomainIsNonDegenerate;
    for (auto [start, end] : toolchain::zip(starts, ends)) {
      auto startValue = getValueOrCreateConstantIndexOp(builder, loc, start);
      auto endValue = getValueOrCreateConstantIndexOp(builder, loc, end);

      // Loop Trip count > 0 iff start < end
      Value dimensionHasNonZeroTripCount = index::CmpOp::create(
          builder, loc, index::IndexCmpPredicate::SLT, startValue, endValue);

      if (!iterationDomainIsNonDegenerate) {
        iterationDomainIsNonDegenerate = dimensionHasNonZeroTripCount;
      } else {
        // Iteration domain is non-degenerate iff all dimensions have loop trip
        // count > 0
        iterationDomainIsNonDegenerate =
            arith::AndIOp::create(builder, loc, iterationDomainIsNonDegenerate,
                                  dimensionHasNonZeroTripCount);
      }
    }

    if (!iterationDomainIsNonDegenerate)
      return;

    auto ifOp = scf::IfOp::create(builder, loc, iterationDomainIsNonDegenerate,
                                  /*withElseRegion=*/false);
    builder.setInsertionPointToStart(&ifOp.getThenRegion().front());

    // Subtract one from the loop ends before composing with the indexing map
    transform(ends, ends.begin(), [&](OpFoldResult end) {
      auto endValue = getValueOrCreateConstantIndexOp(builder, loc, end);
      return builder.createOrFold<index::SubOp>(loc, endValue, one);
    });

    for (OpOperand &opOperand : linalgOp->getOpOperands()) {
      AffineMap indexingMap = linalgOp.getMatchingIndexingMap(&opOperand);
      auto startIndices = affine::makeComposedFoldedMultiResultAffineApply(
          builder, loc, indexingMap, starts);
      auto endIndices = affine::makeComposedFoldedMultiResultAffineApply(
          builder, loc, indexingMap, ends);

      for (auto dim : toolchain::seq(linalgOp.getRank(&opOperand))) {
        auto startIndex =
            getValueOrCreateConstantIndexOp(builder, loc, startIndices[dim]);
        auto endIndex =
            getValueOrCreateConstantIndexOp(builder, loc, endIndices[dim]);

        // Generate:
        //   minIndex = min(startIndex, endIndex)
        //   assert(minIndex >= 0)
        // To ensure we do not generate a negative index. We take the minimum of
        // the start and end indices in order to handle reverse loops such as
        // `affine_map<(i) -> (3 - i)>`
        auto min =
            builder.createOrFold<index::MinSOp>(loc, startIndex, endIndex);
        auto cmpOp = builder.createOrFold<index::CmpOp>(
            loc, index::IndexCmpPredicate::SGE, min, zero);
        auto msg = generateErrorMessage(
            linalgOp, "unexpected negative result on dimension #" +
                          std::to_string(dim) + " of input/output operand #" +
                          std::to_string(opOperand.getOperandNumber()));
        builder.createOrFold<cf::AssertOp>(loc, cmpOp, msg);

        // Generate:
        //   inferredDimSize = max(startIndex, endIndex) + 1
        //   actualDimSize = dim(operand)
        //   assert(inferredDimSize <= actualDimSize)
        // To ensure that we do not index past the bounds of the operands.
        auto max =
            builder.createOrFold<index::MaxSOp>(loc, startIndex, endIndex);

        auto inferredDimSize =
            builder.createOrFold<index::AddOp>(loc, max, one);

        auto actualDimSize =
            createOrFoldDimOp(builder, loc, opOperand.get(), dim);

        // Similar to the verifier, when the affine expression in the indexing
        // map is complicated, we just check that the inferred dimension sizes
        // are in the boundary of the operands' size. Being more precise than
        // that is difficult.
        auto predicate = isa<AffineDimExpr>(indexingMap.getResult(dim))
                             ? index::IndexCmpPredicate::EQ
                             : index::IndexCmpPredicate::SLE;

        cmpOp = builder.createOrFold<index::CmpOp>(
            loc, predicate, inferredDimSize, actualDimSize);
        msg = generateErrorMessage(
            linalgOp, "dimension #" + std::to_string(dim) +
                          " of input/output operand #" +
                          std::to_string(opOperand.getOperandNumber()) +
                          " is incompatible with inferred dimension size");
        builder.createOrFold<cf::AssertOp>(loc, cmpOp, msg);
      }
    }
    builder.setInsertionPointAfter(ifOp);
  }
};

template <typename... OpTs>
void attachInterface(MLIRContext *ctx) {
  (OpTs::template attachInterface<StructuredOpInterface<OpTs>>(*ctx), ...);
}
} // namespace
} // namespace linalg
} // namespace mlir

void mlir::linalg::registerRuntimeVerifiableOpInterfaceExternalModels(
    DialectRegistry &registry) {
  registry.addExtension(+[](MLIRContext *ctx, LinalgDialect *) {
    attachInterface<
#define GET_OP_LIST
#include "mlir/Dialect/Linalg/IR/LinalgStructuredOps.cpp.inc"
        >(ctx);

    // Load additional dialects of which ops may get created.
    ctx->loadDialect<affine::AffineDialect, arith::ArithDialect,
                     cf::ControlFlowDialect, index::IndexDialect,
                     tensor::TensorDialect>();
  });
}
