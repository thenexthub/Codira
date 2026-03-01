//===----------- MultiBuffering.cpp ---------------------------------------===//
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
// This file implements multi buffering transformation.
//
//===----------------------------------------------------------------------===//

#include "mlir/Dialect/Affine/IR/AffineOps.h"
#include "mlir/Dialect/Arith/Utils/Utils.h"
#include "mlir/Dialect/MemRef/IR/MemRef.h"
#include "mlir/Dialect/MemRef/Transforms/Transforms.h"
#include "mlir/IR/AffineExpr.h"
#include "mlir/IR/BuiltinAttributes.h"
#include "mlir/IR/Dominance.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/IR/ValueRange.h"
#include "mlir/Interfaces/LoopLikeInterface.h"
#include "vm/core/ADT/STLExtras.h"
#include "vm/core/Support/Debug.h"

using namespace mlir;

#define DEBUG_TYPE "memref-transforms"
#define DBGS() (toolchain::dbgs() << "[" DEBUG_TYPE "]: ")
#define DBGSNL() (toolchain::dbgs() << "\n")

/// Return true if the op fully overwrite the given `buffer` value.
static bool overrideBuffer(Operation *op, Value buffer) {
  auto copyOp = dyn_cast<memref::CopyOp>(op);
  if (!copyOp)
    return false;
  return copyOp.getTarget() == buffer;
}

/// Replace the uses of `oldOp` with the given `val` and for subview uses
/// propagate the type change. Changing the memref type may require propagating
/// it through subview ops so we cannot just do a replaceAllUse but need to
/// propagate the type change and erase old subview ops.
static void replaceUsesAndPropagateType(RewriterBase &rewriter,
                                        Operation *oldOp, Value val) {
  // Iterate with early_inc to erase current user inside the loop.
  for (OpOperand &use : toolchain::make_early_inc_range(oldOp->getUses())) {
    Operation *user = use.getOwner();
    if (auto subviewUse = dyn_cast<memref::SubViewOp>(user)) {
      // `subview(old_op)` is replaced by a new `subview(val)`.
      OpBuilder::InsertionGuard g(rewriter);
      rewriter.setInsertionPoint(subviewUse);
      MemRefType newType = memref::SubViewOp::inferRankReducedResultType(
          subviewUse.getType().getShape(), cast<MemRefType>(val.getType()),
          subviewUse.getStaticOffsets(), subviewUse.getStaticSizes(),
          subviewUse.getStaticStrides());
      Value newSubview = memref::SubViewOp::create(
          rewriter, subviewUse->getLoc(), newType, val,
          subviewUse.getMixedOffsets(), subviewUse.getMixedSizes(),
          subviewUse.getMixedStrides());

      // Ouch recursion ... is this really necessary?
      replaceUsesAndPropagateType(rewriter, subviewUse, newSubview);

      // Safe to erase.
      rewriter.eraseOp(subviewUse);
      continue;
    }
    // Non-subview: replace with new value.
    rewriter.startOpModification(user);
    use.set(val);
    rewriter.finalizeOpModification(user);
  }
}

// Transformation to do multi-buffering/array expansion to remove dependencies
// on the temporary allocation between consecutive loop iterations.
// Returns success if the transformation happened and failure otherwise.
// This is not a pattern as it requires propagating the new memref type to its
// uses and requires updating subview ops.
FailureOr<memref::AllocOp>
mlir::memref::multiBuffer(RewriterBase &rewriter, memref::AllocOp allocOp,
                          unsigned multiBufferingFactor,
                          bool skipOverrideAnalysis) {
  LLVM_DEBUG(DBGS() << "Start multibuffering: " << allocOp << "\n");
  DominanceInfo dom(allocOp->getParentOp());
  LoopLikeOpInterface candidateLoop;
  for (Operation *user : allocOp->getUsers()) {
    auto parentLoop = user->getParentOfType<LoopLikeOpInterface>();
    if (!parentLoop) {
      if (isa<memref::DeallocOp>(user)) {
        // Allow dealloc outside of any loop.
        // TODO: The whole precondition function here is very brittle and will
        // need to rethought an isolated into a cleaner analysis.
        continue;
      }
      LLVM_DEBUG(DBGS() << "--no parent loop -> fail\n");
      LLVM_DEBUG(DBGS() << "----due to user: " << *user << "\n");
      return failure();
    }
    if (!skipOverrideAnalysis) {
      /// Make sure there is no loop-carried dependency on the allocation.
      if (!overrideBuffer(user, allocOp.getResult())) {
        LLVM_DEBUG(DBGS() << "--Skip user: found loop-carried dependence\n");
        continue;
      }
      // If this user doesn't dominate all the other users keep looking.
      if (toolchain::any_of(allocOp->getUsers(), [&](Operation *otherUser) {
            return !dom.dominates(user, otherUser);
          })) {
        LLVM_DEBUG(
            DBGS() << "--Skip user: does not dominate all other users\n");
        continue;
      }
    } else {
      if (toolchain::any_of(allocOp->getUsers(), [&](Operation *otherUser) {
            return !isa<memref::DeallocOp>(otherUser) &&
                   !parentLoop->isProperAncestor(otherUser);
          })) {
        LLVM_DEBUG(
            DBGS()
            << "--Skip user: not all other users are in the parent loop\n");
        continue;
      }
    }
    candidateLoop = parentLoop;
    break;
  }

  if (!candidateLoop) {
    LLVM_DEBUG(DBGS() << "Skip alloc: no candidate loop\n");
    return failure();
  }

  std::optional<Value> inductionVar = candidateLoop.getSingleInductionVar();
  std::optional<OpFoldResult> lowerBound = candidateLoop.getSingleLowerBound();
  std::optional<OpFoldResult> singleStep = candidateLoop.getSingleStep();
  if (!inductionVar || !lowerBound || !singleStep ||
      !toolchain::hasSingleElement(candidateLoop.getLoopRegions())) {
    LLVM_DEBUG(DBGS() << "Skip alloc: no single iv, lb, step or region\n");
    return failure();
  }

  if (!dom.dominates(allocOp.getOperation(), candidateLoop)) {
    LLVM_DEBUG(DBGS() << "Skip alloc: does not dominate candidate loop\n");
    return failure();
  }

  LLVM_DEBUG(DBGS() << "Start multibuffering loop: " << candidateLoop << "\n");

  // 1. Construct the multi-buffered memref type.
  ArrayRef<int64_t> originalShape = allocOp.getType().getShape();
  SmallVector<int64_t, 4> multiBufferedShape{multiBufferingFactor};
  toolchain::append_range(multiBufferedShape, originalShape);
  LLVM_DEBUG(DBGS() << "--original type: " << allocOp.getType() << "\n");
  MemRefType mbMemRefType = MemRefType::Builder(allocOp.getType())
                                .setShape(multiBufferedShape)
                                .setLayout(MemRefLayoutAttrInterface());
  LLVM_DEBUG(DBGS() << "--multi-buffered type: " << mbMemRefType << "\n");

  // 2. Create the multi-buffered alloc.
  Location loc = allocOp->getLoc();
  OpBuilder::InsertionGuard g(rewriter);
  rewriter.setInsertionPoint(allocOp);
  auto mbAlloc = memref::AllocOp::create(rewriter, loc, mbMemRefType,
                                         ValueRange{}, allocOp->getAttrs());
  LLVM_DEBUG(DBGS() << "--multi-buffered alloc: " << mbAlloc << "\n");

  // 3. Within the loop, build the modular leading index (i.e. each loop
  // iteration %iv accesses slice ((%iv - %lb) / %step) % %mb_factor).
  rewriter.setInsertionPointToStart(
      &candidateLoop.getLoopRegions().front()->front());
  Value ivVal = *inductionVar;
  Value lbVal = getValueOrCreateConstantIndexOp(rewriter, loc, *lowerBound);
  Value stepVal = getValueOrCreateConstantIndexOp(rewriter, loc, *singleStep);
  AffineExpr iv, lb, step;
  bindDims(rewriter.getContext(), iv, lb, step);
  Value bufferIndex = affine::makeComposedAffineApply(
      rewriter, loc, ((iv - lb).floorDiv(step)) % multiBufferingFactor,
      {ivVal, lbVal, stepVal});
  LLVM_DEBUG(DBGS() << "--multi-buffered indexing: " << bufferIndex << "\n");

  // 4. Build the subview accessing the particular slice, taking modular
  // rotation into account.
  int64_t mbMemRefTypeRank = mbMemRefType.getRank();
  IntegerAttr zero = rewriter.getIndexAttr(0);
  IntegerAttr one = rewriter.getIndexAttr(1);
  SmallVector<OpFoldResult> offsets(mbMemRefTypeRank, zero);
  SmallVector<OpFoldResult> sizes(mbMemRefTypeRank, one);
  SmallVector<OpFoldResult> strides(mbMemRefTypeRank, one);
  // Offset is [bufferIndex, 0 ... 0 ].
  offsets.front() = bufferIndex;
  // Sizes is [1, original_size_0 ... original_size_n ].
  for (int64_t i = 0, e = originalShape.size(); i != e; ++i)
    sizes[1 + i] = rewriter.getIndexAttr(originalShape[i]);
  // Strides is [1, 1 ... 1 ].
  MemRefType dstMemref = memref::SubViewOp::inferRankReducedResultType(
      originalShape, mbMemRefType, offsets, sizes, strides);
  Value subview = memref::SubViewOp::create(rewriter, loc, dstMemref, mbAlloc,
                                            offsets, sizes, strides);
  LLVM_DEBUG(DBGS() << "--multi-buffered slice: " << subview << "\n");

  // 5. Due to the recursive nature of replaceUsesAndPropagateType , we need
  // to handle dealloc uses separately..
  for (OpOperand &use : toolchain::make_early_inc_range(allocOp->getUses())) {
    auto deallocOp = dyn_cast<memref::DeallocOp>(use.getOwner());
    if (!deallocOp)
      continue;
    OpBuilder::InsertionGuard g(rewriter);
    rewriter.setInsertionPoint(deallocOp);
    auto newDeallocOp =
        memref::DeallocOp::create(rewriter, deallocOp->getLoc(), mbAlloc);
    (void)newDeallocOp;
    LLVM_DEBUG(DBGS() << "----Created dealloc: " << newDeallocOp << "\n");
    rewriter.eraseOp(deallocOp);
  }

  // 6. RAUW with the particular slice, taking modular rotation into account.
  replaceUsesAndPropagateType(rewriter, allocOp, subview);

  // 7. Finally, erase the old allocOp.
  rewriter.eraseOp(allocOp);

  return mbAlloc;
}

FailureOr<memref::AllocOp>
mlir::memref::multiBuffer(memref::AllocOp allocOp,
                          unsigned multiBufferingFactor,
                          bool skipOverrideAnalysis) {
  IRRewriter rewriter(allocOp->getContext());
  return multiBuffer(rewriter, allocOp, multiBufferingFactor,
                     skipOverrideAnalysis);
}
