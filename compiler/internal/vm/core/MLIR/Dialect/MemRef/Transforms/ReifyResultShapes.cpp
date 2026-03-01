//===- ReifyResultShapes.cpp - Reify result shapes ------------------------===//
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
// This transform reifies result shapes of `ReifyRankedShapedTypeOpInterface`
// operations with ranked `memref` and `tensor` results.
//
//===----------------------------------------------------------------------===//

#include "mlir/Dialect/MemRef/Transforms/Passes.h"

#include "mlir/Dialect/Affine/IR/AffineOps.h"
#include "mlir/Dialect/MemRef/IR/MemRef.h"
#include "mlir/Dialect/MemRef/Transforms/Transforms.h"
#include "mlir/Dialect/Tensor/IR/Tensor.h"
#include "mlir/Interfaces/InferTypeOpInterface.h"
#include "vm/core/Support/InterleavedRange.h"

#define DEBUG_TYPE "reify-result-shapes"
#define DBGS() (toolchain::dbgs() << "[" DEBUG_TYPE << "]: ")

namespace mlir {
namespace memref {
#define GEN_PASS_DEF_REIFYRESULTSHAPESPASS
#include "mlir/Dialect/MemRef/Transforms/Passes.h.inc"
} // namespace memref
} // namespace mlir

using namespace mlir;

/// Reifies the results of `op`, potentially replacing `op` with a reified
/// version. Returns `failure` if `mlir::reifyResultShapes` returned failure,
/// otherwise it always succeeds. Users of this transform should always expect
/// it to modify the IR, even when it fails. If any of the result types changes,
/// the transform will insert cast operations to the old type to keep the IR
/// consistent.
static LogicalResult reifyOpResultShapes(RewriterBase &rewriter,
                                         ReifyRankedShapedTypeOpInterface op) {
  LLVM_DEBUG({ DBGS() << " reifying op: " << op << "\n"; });
  // Get the reified out shapes.
  ReifiedRankedShapedTypeDims reifiedResultShapes;
  if (failed(mlir::reifyResultShapes(rewriter, op, reifiedResultShapes)) ||
      reifiedResultShapes.empty()) {
    return op->emitWarning() << "failed to get the reified shapes";
  }

  bool modified = false;
  // Compute the new output types.
  SmallVector<Type> outTypes;
  for (const auto &[oldTy, reifiedShape] :
       toolchain::zip(op->getResultTypes(), reifiedResultShapes)) {
    // Skip if it's not a memref or tensor type.
    if (!isa<RankedTensorType, MemRefType>(oldTy)) {
      outTypes.push_back(oldTy);
      continue;
    }

    ShapedType shapedTy = dyn_cast<ShapedType>(oldTy);

    SmallVector<int64_t> shape = toolchain::to_vector(shapedTy.getShape());
    for (auto &&[dim, ofr] : toolchain::zip_equal(shape, reifiedShape)) {
      std::optional<int64_t> maybeCst = getConstantIntValue(ofr);
      // If the reified dim is dynamic set it appropriately.
      if (!maybeCst.has_value()) {
        dim = ShapedType::kDynamic;
        continue;
      }
      // Set the static dim.
      dim = *maybeCst;
    }

    // If the shape didn't change continue.
    if (shape == shapedTy.getShape()) {
      outTypes.push_back(oldTy);
      continue;
    }
    modified = true;
    outTypes.push_back(shapedTy.cloneWith(shape, shapedTy.getElementType()));
  }

  // Return if we don't need to update.
  if (!modified) {
    LLVM_DEBUG({ DBGS() << "- op doesn't require update\n"; });
    return success();
  }

  LLVM_DEBUG({
    DBGS() << "- oldTypes: " << toolchain::interleaved_array(op->getResultTypes())
           << " \n";
    DBGS() << "- outTypes: " << toolchain::interleaved_array(outTypes) << " \n";
  });

  // We now have outTypes that need to be turned to cast ops.
  Location loc = op->getLoc();
  SmallVector<Value> newResults;
  // TODO: `mlir::reifyResultShapes` and op verifiers may not agree atm.
  // This is a confluence problem that will need to be addressed.
  // For now, we know PadOp and ConcatOp are fine.
  assert((isa<tensor::PadOp, tensor::ConcatOp>(op.getOperation())) &&
         "incorrect op");
  Operation *newOp = rewriter.clone(*op);
  for (auto [reifiedTy, oldRes] : toolchain::zip(outTypes, op->getResults())) {
    OpResult newRes = newOp->getResult(oldRes.getResultNumber());
    Type oldTy = oldRes.getType();
    // Continue if the type remained invariant or is not shaped.
    if (oldTy == reifiedTy || !isa<MemRefType, RankedTensorType>(oldTy)) {
      newResults.push_back(newRes);
      continue;
    }

    // Update the type.
    newRes.setType(reifiedTy);
    if (isa<RankedTensorType>(reifiedTy)) {
      newResults.push_back(
          tensor::CastOp::create(rewriter, loc, oldTy, newRes));
    } else {
      assert(isa<MemRefType>(reifiedTy) && "expected a memref type");
      newResults.push_back(
          memref::CastOp::create(rewriter, loc, oldTy, newRes));
    }
  }

  LLVM_DEBUG({
    DBGS() << "- reified results " << toolchain::interleaved_array(newResults)
           << "\n";
  });
  rewriter.replaceOp(op, newResults);
  return success();
}

//===----------------------------------------------------------------------===//
// Pass registration
//===----------------------------------------------------------------------===//

namespace {
struct ReifyResultShapesPass final
    : public memref::impl::ReifyResultShapesPassBase<ReifyResultShapesPass> {
  void runOnOperation() override;
};
} // namespace

void ReifyResultShapesPass::runOnOperation() {
  SmallVector<ReifyRankedShapedTypeOpInterface> ops;
  getOperation()->walk([&](ReifyRankedShapedTypeOpInterface op) {
    // Handle ops that are not DPS and that do not carry an tied operand shapes.
    // For now, limit to tensor::PadOp and tensor::ConcatOp.
    if (!isa<tensor::PadOp, tensor::ConcatOp>(op.getOperation()))
      return;
    ops.push_back(op);
  });
  IRRewriter rewriter(&getContext());
  for (ReifyRankedShapedTypeOpInterface op : ops) {
    rewriter.setInsertionPoint(op);
    (void)reifyOpResultShapes(rewriter, op);
  }
}
