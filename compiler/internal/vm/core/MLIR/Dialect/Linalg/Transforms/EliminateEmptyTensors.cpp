//===- EmptyTensorElimination.cpp - tensor.empty op elimination -----------===//
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

#include "mlir/Dialect/Linalg/Transforms/Transforms.h"

#include "mlir/Dialect/Bufferization/IR/BufferizableOpInterface.h"
#include "mlir/Dialect/Bufferization/Transforms/OneShotAnalysis.h"
#include "mlir/Dialect/Bufferization/Transforms/Transforms.h"
#include "mlir/Dialect/Linalg/IR/Linalg.h"
#include "mlir/Dialect/Tensor/IR/Tensor.h"

using namespace mlir;
using namespace mlir::bufferization;
using namespace mlir::linalg;

/// Get an output operand that matches the given input operand and can be used
/// to eliminate a tensor.empty op.
static OpOperand *getUnusedOutOperand(LinalgOp op, OpOperand *in) {
  for (OpOperand &operand : op.getDpsInitsMutable()) {
    // Operand must be unused.
    if (op.payloadUsesValueFromOperand(&operand))
      continue;
    // Types must match.
    if (operand.get().getType() != in->get().getType())
      continue;
    // Indexing maps must match.
    if (op.getMatchingIndexingMap(&operand) != op.getMatchingIndexingMap(in))
      continue;
    return &operand;
  }
  return nullptr;
}

LogicalResult linalg::linalgOpAnchoredEmptyTensorEliminationStep(
    RewriterBase &rewriter, Operation *op, OneShotAnalysisState &state) {
  OpBuilder::InsertionGuard g(rewriter);
  DominanceInfo domInfo;

  op->walk([&](LinalgOp op) {
    // Only ops with all "parallel" iterator types are supported.
    if (op.getNumParallelLoops() != op.getNumLoops())
      return WalkResult::skip();

    for (OpOperand *in : op.getDpsInputOperands()) {
      // Skip non-tensor operands.
      if (!isa<RankedTensorType>(in->get().getType()))
        continue;

      // Find tensor.empty ops on the reverse SSA use-def chain. Only follow
      // equivalent tensors. I.e., stop when there are ops such as extract_slice
      // on the path.
      TraversalConfig config;
      config.followEquivalentOnly = true;
      config.alwaysIncludeLeaves = false;
      SetVector<Value> emptyTensors = state.findValueInReverseUseDefChain(
          in, /*condition=*/
          [&](Value val) {
            return val.getDefiningOp<tensor::EmptyOp>() &&
                   val.getType() == in->get().getType();
          },
          config);
      if (emptyTensors.empty())
        continue;

      // Find matching out operand.
      OpOperand *out = getUnusedOutOperand(op, in);
      if (!out)
        continue;

      // Check if this transform would violate dominance.
      if (!toolchain::all_of(emptyTensors, [&](Value v) {
            return domInfo.properlyDominates(out->get(), v.getDefiningOp());
          }))
        continue;

      // Replace all uses of the tensor.empty, but do not delete it yet. It will
      // fold away later (to not invalidate DominanceInfo).
      for (Value v : emptyTensors) {
        assert(v.getDefiningOp<tensor::EmptyOp>() && "expected tensor.empty");
        rewriter.replaceAllUsesWith(v, out->get());
      }

      // Turn the "in" into an "out".
      rewriter.modifyOpInPlace(op, [&]() {
        out->set(in->get());
        // The original "in" could be removed entirely here (because it will no
        // longer have any uses in the payload), but we delegate this to
        // existing cleanup patterns that remove unused operands.
        in->set(emptyTensors.front());
        BlockArgument outArg = op.getMatchingBlockArgument(out);
        assert(outArg.getUses().empty() && "expected that out has no uses");
        BlockArgument inArg = op.getMatchingBlockArgument(in);
        rewriter.replaceAllUsesWith(inArg, outArg);
        assert(!op.payloadUsesValueFromOperand(in) &&
               "expected that the in operand is now unused");
      });

      state.resetCache();
    }

    return WalkResult::advance();
  });
  return success();
}
