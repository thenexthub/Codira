//===- Interchange.cpp - Linalg interchange transformation ----------------===//
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
// This file implements the linalg interchange transformation.
//
//===----------------------------------------------------------------------===//

#include "mlir/Dialect/Affine/IR/AffineOps.h"
#include "mlir/Dialect/Linalg/IR/Linalg.h"
#include "mlir/Dialect/Linalg/Transforms/Transforms.h"
#include "mlir/Dialect/Linalg/Utils/Utils.h"
#include "mlir/Dialect/Utils/IndexingUtils.h"
#include "mlir/Dialect/Utils/StructuredOpsUtils.h"
#include "mlir/Dialect/Vector/IR/VectorOps.h"
#include "mlir/IR/AffineExpr.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Support/LLVM.h"
#include "vm/core/ADT/ScopeExit.h"

#define DEBUG_TYPE "linalg-interchange"

using namespace mlir;
using namespace mlir::linalg;

static LogicalResult
interchangeGenericOpPrecondition(GenericOp genericOp,
                                 ArrayRef<unsigned> interchangeVector) {
  // Interchange vector must be non-empty and match the number of loops.
  if (interchangeVector.empty() ||
      genericOp.getNumLoops() != interchangeVector.size())
    return failure();
  // Permutation map must be invertible.
  if (!inversePermutation(AffineMap::getPermutationMap(interchangeVector,
                                                       genericOp.getContext())))
    return failure();
  return success();
}

FailureOr<GenericOp>
mlir::linalg::interchangeGenericOp(RewriterBase &rewriter, GenericOp genericOp,
                                   ArrayRef<unsigned> interchangeVector) {
  if (failed(interchangeGenericOpPrecondition(genericOp, interchangeVector)))
    return rewriter.notifyMatchFailure(genericOp, "preconditions not met");

  // 1. Compute the inverse permutation map, it must be non-null since the
  // preconditions are satisfied.
  MLIRContext *context = genericOp.getContext();
  AffineMap permutationMap = inversePermutation(
      AffineMap::getPermutationMap(interchangeVector, context));
  assert(permutationMap && "unexpected null map");

  // Start a guarded inplace update.
  rewriter.startOpModification(genericOp);
  toolchain::scope_exit guard([&]() { rewriter.finalizeOpModification(genericOp); });

  // 2. Compute the interchanged indexing maps.
  SmallVector<AffineMap> newIndexingMaps;
  for (OpOperand &opOperand : genericOp->getOpOperands()) {
    AffineMap m = genericOp.getMatchingIndexingMap(&opOperand);
    if (!permutationMap.isEmpty())
      m = m.compose(permutationMap);
    newIndexingMaps.push_back(m);
  }
  genericOp.setIndexingMapsAttr(
      rewriter.getAffineMapArrayAttr(newIndexingMaps));

  // 3. Compute the interchanged iterator types.
  ArrayRef<Attribute> itTypes = genericOp.getIteratorTypes().getValue();
  SmallVector<Attribute> itTypesVector;
  toolchain::append_range(itTypesVector, itTypes);
  SmallVector<int64_t> permutation(interchangeVector);
  applyPermutationToVector(itTypesVector, permutation);
  genericOp.setIteratorTypesAttr(rewriter.getArrayAttr(itTypesVector));

  // 4. Transform the index operations by applying the permutation map.
  if (genericOp.hasIndexSemantics()) {
    OpBuilder::InsertionGuard guard(rewriter);
    for (IndexOp indexOp :
         toolchain::make_early_inc_range(genericOp.getBody()->getOps<IndexOp>())) {
      rewriter.setInsertionPoint(indexOp);
      SmallVector<Value> allIndices;
      allIndices.reserve(genericOp.getNumLoops());
      toolchain::transform(toolchain::seq<uint64_t>(0, genericOp.getNumLoops()),
                      std::back_inserter(allIndices), [&](uint64_t dim) {
                        return IndexOp::create(rewriter, indexOp->getLoc(),
                                               dim);
                      });
      rewriter.replaceOpWithNewOp<affine::AffineApplyOp>(
          indexOp, permutationMap.getSubMap(indexOp.getDim()), allIndices);
    }
  }

  return genericOp;
}
