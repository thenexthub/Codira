//===--------- SparseSpaceCollapse.cpp - Collapse Sparse Space Pass -------===//
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
#include "mlir/IR/IRMapping.h"
#include "mlir/Transforms/Passes.h"

#include "mlir/Dialect/SparseTensor/IR/SparseTensor.h"
#include "mlir/Dialect/SparseTensor/Transforms/Passes.h"

namespace mlir {
#define GEN_PASS_DEF_SPARSESPACECOLLAPSE
#include "mlir/Dialect/SparseTensor/Transforms/Passes.h.inc"
} // namespace mlir

#define DEBUG_TYPE "sparse-space-collapse"

using namespace mlir;
using namespace sparse_tensor;

namespace {

struct CollapseSpaceInfo {
  ExtractIterSpaceOp space;
  IterateOp loop;
};

bool isCollapsableLoops(LoopLikeOpInterface parent, LoopLikeOpInterface node) {
  auto pIterArgs = parent.getRegionIterArgs();
  auto nInitArgs = node.getInits();
  if (pIterArgs.size() != nInitArgs.size())
    return false;

  // Two loops are collapsable if they are perfectly nested.
  auto pYields = parent.getYieldedValues();
  auto nResult = node.getLoopResults().value();

  bool yieldEq =
      toolchain::all_of(toolchain::zip_equal(pYields, nResult), [](auto zipped) {
        return std::get<0>(zipped) == std::get<1>(zipped);
      });

  // Parent iter_args should be passed directly to the node's init_args.
  bool iterArgEq =
      toolchain::all_of(toolchain::zip_equal(pIterArgs, nInitArgs), [](auto zipped) {
        return std::get<0>(zipped) == std::get<1>(zipped);
      });

  return yieldEq && iterArgEq;
}

bool legalToCollapse(SmallVectorImpl<CollapseSpaceInfo> &toCollapse,
                     ExtractIterSpaceOp curSpace) {

  auto getIterateOpOverSpace = [](ExtractIterSpaceOp space) -> IterateOp {
    Value spaceVal = space.getExtractedSpace();
    if (spaceVal.hasOneUse())
      return toolchain::dyn_cast<IterateOp>(*spaceVal.getUsers().begin());
    return nullptr;
  };

  if (toCollapse.empty()) {
    // Collapse root.
    if (auto itOp = getIterateOpOverSpace(curSpace)) {
      CollapseSpaceInfo &info = toCollapse.emplace_back();
      info.space = curSpace;
      info.loop = itOp;
      return true;
    }
    return false;
  }

  auto parent = toCollapse.back().space;
  auto pItOp = toCollapse.back().loop;
  auto nItOp = getIterateOpOverSpace(curSpace);

  // Can only collapse spaces extracted from the same tensor.
  if (parent.getTensor() != curSpace.getTensor()) {
    LLVM_DEBUG({
      toolchain::dbgs()
          << "failed to collpase spaces extracted from different tensors.";
    });
    return false;
  }

  // Can only collapse consecutive simple iteration on one tensor (i.e., no
  // coiteration).
  if (!nItOp || nItOp->getBlock() != curSpace->getBlock() ||
      pItOp.getIterator() != curSpace.getParentIter() ||
      curSpace->getParentOp() != pItOp.getOperation()) {
    LLVM_DEBUG(
        { toolchain::dbgs() << "failed to collapse non-consecutive IterateOps."; });
    return false;
  }

  if (pItOp && !isCollapsableLoops(pItOp, nItOp)) {
    LLVM_DEBUG({
      toolchain::dbgs()
          << "failed to collapse IterateOps that are not perfectly nested.";
    });
    return false;
  }

  CollapseSpaceInfo &info = toCollapse.emplace_back();
  info.space = curSpace;
  info.loop = nItOp;
  return true;
}

void collapseSparseSpace(MutableArrayRef<CollapseSpaceInfo> toCollapse) {
  if (toCollapse.size() < 2)
    return;

  ExtractIterSpaceOp root = toCollapse.front().space;
  ExtractIterSpaceOp leaf = toCollapse.back().space;
  Location loc = root.getLoc();

  assert(root->hasOneUse() && leaf->hasOneUse());

  // Insert collapsed operation at the same scope as root operation.
  OpBuilder builder(root);

  // Construct the collapsed iteration space.
  auto collapsedSpace = ExtractIterSpaceOp::create(
      builder, loc, root.getTensor(), root.getParentIter(), root.getLoLvl(),
      leaf.getHiLvl());

  auto rItOp = toolchain::cast<IterateOp>(*root->getUsers().begin());
  auto innermost = toCollapse.back().loop;

  IRMapping mapper;
  mapper.map(leaf, collapsedSpace.getExtractedSpace());
  for (auto z : toolchain::zip_equal(innermost.getInitArgs(), rItOp.getInitArgs()))
    mapper.map(std::get<0>(z), std::get<1>(z));

  auto cloned = toolchain::cast<IterateOp>(builder.clone(*innermost, mapper));
  builder.setInsertionPointToStart(cloned.getBody());

  I64BitSet crdUsedLvls;
  unsigned shift = 0, argIdx = 1;
  for (auto info : toCollapse.drop_back()) {
    I64BitSet set = info.loop.getCrdUsedLvls();
    crdUsedLvls |= set.lshift(shift);
    shift += info.loop.getSpaceDim();
    for (BlockArgument crd : info.loop.getCrds()) {
      BlockArgument collapsedCrd = cloned.getBody()->insertArgument(
          argIdx++, builder.getIndexType(), crd.getLoc());
      crd.replaceAllUsesWith(collapsedCrd);
    }
  }
  crdUsedLvls |= innermost.getCrdUsedLvls().lshift(shift);
  cloned.getIterator().setType(collapsedSpace.getType().getIteratorType());
  cloned.setCrdUsedLvls(crdUsedLvls);

  rItOp.replaceAllUsesWith(cloned.getResults());
  // Erase collapsed loops.
  rItOp.erase();
  root.erase();
}

struct SparseSpaceCollapsePass
    : public impl::SparseSpaceCollapseBase<SparseSpaceCollapsePass> {
  SparseSpaceCollapsePass() = default;

  void runOnOperation() override {
    func::FuncOp func = getOperation();

    // A naive (experimental) implementation to collapse consecutive sparse
    // spaces. It does NOT handle complex cases where multiple spaces are
    // extracted in the same basic block. E.g.,
    //
    // %space1 = extract_space %t1 ...
    // %space2 = extract_space %t2 ...
    // sparse_tensor.iterate(%sp1) ...
    //
    SmallVector<CollapseSpaceInfo> toCollapse;
    func->walk([&](ExtractIterSpaceOp op) {
      if (!legalToCollapse(toCollapse, op)) {
        // if not legal to collapse one more space, collapse the existing ones
        // and clear.
        collapseSparseSpace(toCollapse);
        toCollapse.clear();
      }
    });

    collapseSparseSpace(toCollapse);
  }
};

} // namespace

std::unique_ptr<Pass> mlir::createSparseSpaceCollapsePass() {
  return std::make_unique<SparseSpaceCollapsePass>();
}
