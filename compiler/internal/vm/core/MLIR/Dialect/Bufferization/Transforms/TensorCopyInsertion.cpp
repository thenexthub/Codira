//===- TensorCopyInsertion.cpp - Resolve Bufferization Conflicts w/ Copies ===//
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

#include "mlir/Dialect/Bufferization/Transforms/Passes.h"

#include "mlir/Dialect/Bufferization/IR/BufferizableOpInterface.h"
#include "mlir/Dialect/Bufferization/Transforms/Bufferize.h"
#include "mlir/Dialect/Bufferization/Transforms/OneShotAnalysis.h"
#include "mlir/Dialect/Bufferization/Transforms/OneShotModuleBufferize.h"
#include "mlir/Dialect/Bufferization/Transforms/Transforms.h"

using namespace mlir;
using namespace mlir::bufferization;

LogicalResult mlir::bufferization::insertTensorCopies(
    Operation *op, const OneShotBufferizationOptions &options,
    const BufferizationState &bufferizationState,
    BufferizationStatistics *statistics) {
  OneShotAnalysisState analysisState(op, options);
  // Run normal One-Shot Bufferize analysis or One-Shot Module Bufferize
  // analysis depending on whether function boundary bufferization is enabled or
  // not.
  if (options.bufferizeFunctionBoundaries) {
    if (failed(analyzeModuleOp(op, analysisState, statistics)))
      return failure();
  } else {
    if (failed(analyzeOp(op, analysisState, statistics)))
      return failure();
  }

  if (options.testAnalysisOnly)
    return success();

  return insertTensorCopies(op, analysisState, bufferizationState);
}

LogicalResult mlir::bufferization::insertTensorCopies(
    Operation *op, const AnalysisState &analysisState,
    const BufferizationState &bufferizationState) {
  IRRewriter rewriter(op->getContext());

  // It may be more efficient to walk in pre-order here, but the current
  // implementation visits regions of ops even if they are not allowed or
  // bufferizable, and existing tests rely on this behavior.
  // For now, only exclude nested operations if they are in a different symbol
  // table scope.
  WalkResult result = op->walk([&](Operation *nestedOp) {
    if (op->hasTrait<OpTrait::SymbolTable>() &&
        nestedOp->getParentWithTrait<OpTrait::SymbolTable>() != op)
      return WalkResult::skip();

    auto bufferizableOp =
        analysisState.getOptions().dynCastBufferizableOp(nestedOp);
    if (!bufferizableOp)
      return WalkResult::skip();

    // Find inplacability conflicts and resolve them. (Typically with explicit
    // tensor copies in the form of AllocTensorOps.)
    rewriter.setInsertionPoint(nestedOp);
    if (failed(bufferizableOp.resolveConflicts(rewriter, analysisState,
                                               bufferizationState)))
      return WalkResult::interrupt();

    return WalkResult::advance();
  });

  return failure(result.wasInterrupted());
}
