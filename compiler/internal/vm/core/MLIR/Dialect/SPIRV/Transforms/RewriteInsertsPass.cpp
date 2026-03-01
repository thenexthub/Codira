//===- RewriteInsertsPass.cpp - MLIR conversion pass ----------------------===//
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
// This file implements a pass to rewrite sequential chains of
// `spirv::CompositeInsert` operations into `spirv::CompositeConstruct`
// operations.
//
//===----------------------------------------------------------------------===//

#include "mlir/Dialect/SPIRV/Transforms/Passes.h"

#include "mlir/Dialect/SPIRV/IR/SPIRVOps.h"
#include "mlir/IR/Builders.h"

namespace mlir {
namespace spirv {
#define GEN_PASS_DEF_SPIRVREWRITEINSERTSPASS
#include "mlir/Dialect/SPIRV/Transforms/Passes.h.inc"
} // namespace spirv
} // namespace mlir

using namespace mlir;

namespace {

/// Replaces sequential chains of `spirv::CompositeInsertOp` operation into
/// `spirv::CompositeConstructOp` operation if possible.
class RewriteInsertsPass
    : public spirv::impl::SPIRVRewriteInsertsPassBase<RewriteInsertsPass> {
public:
  void runOnOperation() override;

private:
  /// Collects a sequential insertion chain by the given
  /// `spirv::CompositeInsertOp` operation, if the given operation is the last
  /// in the chain.
  LogicalResult
  collectInsertionChain(spirv::CompositeInsertOp op,
                        SmallVectorImpl<spirv::CompositeInsertOp> &insertions);
};

} // namespace

void RewriteInsertsPass::runOnOperation() {
  SmallVector<SmallVector<spirv::CompositeInsertOp, 4>, 4> workList;
  getOperation().walk([this, &workList](spirv::CompositeInsertOp op) {
    SmallVector<spirv::CompositeInsertOp, 4> insertions;
    if (succeeded(collectInsertionChain(op, insertions)))
      workList.push_back(insertions);
  });

  for (const auto &insertions : workList) {
    auto lastCompositeInsertOp = insertions.back();
    auto compositeType = lastCompositeInsertOp.getType();
    auto location = lastCompositeInsertOp.getLoc();

    SmallVector<Value, 4> operands;
    // Collect inserted objects.
    for (auto insertionOp : insertions)
      operands.push_back(insertionOp.getObject());

    OpBuilder builder(lastCompositeInsertOp);
    auto compositeConstructOp = spirv::CompositeConstructOp::create(
        builder, location, compositeType, operands);

    lastCompositeInsertOp.replaceAllUsesWith(
        compositeConstructOp->getResult(0));

    // Erase ops.
    for (auto insertOp : toolchain::reverse(insertions)) {
      auto *op = insertOp.getOperation();
      if (op->use_empty())
        insertOp.erase();
    }
  }
}

LogicalResult RewriteInsertsPass::collectInsertionChain(
    spirv::CompositeInsertOp op,
    SmallVectorImpl<spirv::CompositeInsertOp> &insertions) {
  if (isa<spirv::CooperativeMatrixType>(op.getComposite().getType()))
    return failure();

  auto indicesArrayAttr = cast<ArrayAttr>(op.getIndices());
  // TODO: handle nested composite object.
  if (indicesArrayAttr.size() == 1) {
    auto numElements = cast<spirv::CompositeType>(op.getComposite().getType())
                           .getNumElements();

    auto index = cast<IntegerAttr>(indicesArrayAttr[0]).getInt();
    // Need a last index to collect a sequential chain.
    if (index + 1 != numElements)
      return failure();

    insertions.resize(numElements);
    while (true) {
      insertions[index] = op;

      if (index == 0)
        return success();

      op = op.getComposite().getDefiningOp<spirv::CompositeInsertOp>();
      if (!op)
        return failure();

      --index;
      indicesArrayAttr = cast<ArrayAttr>(op.getIndices());
      if ((indicesArrayAttr.size() != 1) ||
          (cast<IntegerAttr>(indicesArrayAttr[0]).getInt() != index))
        return failure();
    }
  }
  return failure();
}
