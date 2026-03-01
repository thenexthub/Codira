//===- BufferizableOpInterfaceImpl.cpp - Impl. of BufferizableOpInterface -===//
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

#include "mlir/Dialect/Shape/Transforms/BufferizableOpInterfaceImpl.h"

#include "mlir/Dialect/Bufferization/IR/BufferizableOpInterface.h"
#include "mlir/Dialect/Bufferization/IR/Bufferization.h"
#include "mlir/Dialect/Shape/IR/Shape.h"
#include "mlir/IR/Operation.h"
#include "mlir/IR/PatternMatch.h"

using namespace mlir;
using namespace mlir::bufferization;
using namespace mlir::shape;

namespace mlir {
namespace shape {
namespace {

/// Bufferization of shape.assuming.
struct AssumingOpInterface
    : public BufferizableOpInterface::ExternalModel<AssumingOpInterface,
                                                    shape::AssumingOp> {
  AliasingOpOperandList
  getAliasingOpOperands(Operation *op, Value value,
                        const AnalysisState &state) const {
    // AssumingOps do not have tensor OpOperands. The yielded value can be any
    // SSA value that is in scope. To allow for use-def chain traversal through
    // AssumingOps in the analysis, the corresponding yield value is considered
    // to be aliasing with the result.
    auto assumingOp = cast<shape::AssumingOp>(op);
    size_t resultNum = std::distance(op->getOpResults().begin(),
                                     toolchain::find(op->getOpResults(), value));
    // TODO: Support multiple blocks.
    assert(assumingOp.getDoRegion().hasOneBlock() &&
           "expected exactly 1 block");
    auto yieldOp = dyn_cast<shape::AssumingYieldOp>(
        assumingOp.getDoRegion().front().getTerminator());
    assert(yieldOp && "expected shape.assuming_yield terminator");
    return {{&yieldOp->getOpOperand(resultNum), BufferRelation::Equivalent}};
  }

  LogicalResult bufferize(Operation *op, RewriterBase &rewriter,
                          const BufferizationOptions &options,
                          BufferizationState &state) const {
    auto assumingOp = cast<shape::AssumingOp>(op);
    assert(assumingOp.getDoRegion().hasOneBlock() && "only 1 block supported");
    auto yieldOp = cast<shape::AssumingYieldOp>(
        assumingOp.getDoRegion().front().getTerminator());

    // Create new op and move over region.
    TypeRange newResultTypes(yieldOp.getOperands());
    auto newOp = shape::AssumingOp::create(
        rewriter, op->getLoc(), newResultTypes, assumingOp.getWitness());
    newOp.getDoRegion().takeBody(assumingOp.getRegion());

    // Update all uses of the old op.
    rewriter.setInsertionPointAfter(newOp);
    SmallVector<Value> newResults;
    for (const auto &it : toolchain::enumerate(assumingOp->getResultTypes())) {
      if (isa<TensorType>(it.value())) {
        newResults.push_back(bufferization::ToTensorOp::create(
            rewriter, assumingOp.getLoc(), it.value(),
            newOp->getResult(it.index())));
      } else {
        newResults.push_back(newOp->getResult(it.index()));
      }
    }

    // Replace old op.
    rewriter.replaceOp(assumingOp, newResults);

    return success();
  }
};

/// Bufferization of shape.assuming_yield. Bufferized as part of their enclosing
/// ops, so this is for analysis only.
struct AssumingYieldOpInterface
    : public BufferizableOpInterface::ExternalModel<AssumingYieldOpInterface,
                                                    shape::AssumingYieldOp> {
  bool bufferizesToMemoryRead(Operation *op, OpOperand &opOperand,
                              const AnalysisState &state) const {
    return true;
  }

  bool bufferizesToMemoryWrite(Operation *op, OpOperand &opOperand,
                               const AnalysisState &state) const {
    return false;
  }

  AliasingValueList getAliasingValues(Operation *op, OpOperand &opOperand,
                                      const AnalysisState &state) const {
    assert(isa<shape::AssumingOp>(op->getParentOp()) &&
           "expected that parent is an AssumingOp");
    OpResult opResult =
        op->getParentOp()->getResult(opOperand.getOperandNumber());
    return {{opResult, BufferRelation::Equivalent}};
  }

  bool mustBufferizeInPlace(Operation *op, OpOperand &opOperand,
                            const AnalysisState &state) const {
    // Yield operands always bufferize inplace. Otherwise, an alloc + copy
    // may be generated inside the block. We should not return/yield allocations
    // when possible.
    return true;
  }

  LogicalResult bufferize(Operation *op, RewriterBase &rewriter,
                          const BufferizationOptions &options,
                          BufferizationState &state) const {
    auto yieldOp = cast<shape::AssumingYieldOp>(op);
    SmallVector<Value> newResults;
    for (Value value : yieldOp.getOperands()) {
      if (isa<TensorType>(value.getType())) {
        FailureOr<Value> buffer = getBuffer(rewriter, value, options, state);
        if (failed(buffer))
          return failure();
        newResults.push_back(*buffer);
      } else {
        newResults.push_back(value);
      }
    }
    replaceOpWithNewBufferizedOp<shape::AssumingYieldOp>(rewriter, op,
                                                         newResults);
    return success();
  }
};

} // namespace
} // namespace shape
} // namespace mlir

void mlir::shape::registerBufferizableOpInterfaceExternalModels(
    DialectRegistry &registry) {
  registry.addExtension(+[](MLIRContext *ctx, shape::ShapeDialect *dialect) {
    shape::AssumingOp::attachInterface<AssumingOpInterface>(*ctx);
    shape::AssumingYieldOp::attachInterface<AssumingYieldOpInterface>(*ctx);
  });
}
