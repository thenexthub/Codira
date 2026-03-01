//===- ShardingInterface.cpp -------------------------------------*- C++-*-===//
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

#include "mlir/Dialect/Shard/Interfaces/ShardingInterface.h"
#include "mlir/Dialect/Shard/Interfaces/ShardingInterfaceImpl.h"

#include "mlir/Dialect/Shard/IR/ShardOps.h"
#include "mlir/IR/AffineMap.h"
#include "mlir/IR/IRMapping.h"
#include "mlir/Support/LLVM.h"
#include "vm/core/ADT/ArrayRef.h"
#include "vm/core/ADT/STLExtras.h"
#include "vm/core/ADT/SmallSet.h"
#include "vm/core/Support/Debug.h"

#include <utility>

#define DEBUG_TYPE "sharding-interface"
#define DBGS() (toolchain::dbgs() << "[" DEBUG_TYPE << "]: ")

using namespace mlir;
using namespace mlir::shard;

#include "mlir/Dialect/Shard/Interfaces/ShardingInterface.cpp.inc"

//===----------------------------------------------------------------------===//
// common util functions
//===----------------------------------------------------------------------===//

static LogicalResult
checkOperandAffineExprRecursively(AffineExpr expr,
                                  SmallVectorImpl<bool> &seenIds) {
  switch (expr.getKind()) {
  case AffineExprKind::Add: {
    auto binOpExpr = cast<AffineBinaryOpExpr>(expr);
    AffineExpr lhs = binOpExpr.getLHS();
    AffineExpr rhs = binOpExpr.getRHS();
    if (failed(checkOperandAffineExprRecursively(lhs, seenIds)))
      return failure();
    if (failed(checkOperandAffineExprRecursively(rhs, seenIds)))
      return failure();
    return success();
  }
  case AffineExprKind::Mul: {
    auto binOpExpr = cast<AffineBinaryOpExpr>(expr);
    AffineExpr lhs = binOpExpr.getLHS();
    AffineExpr rhs = binOpExpr.getRHS();
    AffineExpr dimExpr;
    if (lhs.getKind() == AffineExprKind::DimId &&
        rhs.getKind() == AffineExprKind::Constant) {
      dimExpr = lhs;
    } else if (rhs.getKind() == AffineExprKind::DimId &&
               lhs.getKind() == AffineExprKind::Constant) {
      dimExpr = rhs;
    } else {
      return failure();
    }
    unsigned position = cast<AffineDimExpr>(dimExpr).getPosition();
    if ((size_t)position >= seenIds.size() || seenIds[position])
      return failure();
    seenIds[position] = true;
    return success();
  }
  case AffineExprKind::DimId: {
    unsigned position = cast<AffineDimExpr>(expr).getPosition();
    if ((size_t)position >= seenIds.size() || seenIds[position])
      return failure();
    seenIds[position] = true;
    return success();
  }
  default:
    return failure();
  }
}

static FailureOr<toolchain::SmallSet<unsigned, 2>>
checkOperandAffineExpr(AffineExpr expr, unsigned numDims) {
  SmallVector<bool> seenIds(numDims, false);
  if (failed(checkOperandAffineExprRecursively(expr, seenIds)))
    return failure();

  toolchain::SmallSet<unsigned, 2> positions;
  for (auto it : toolchain::enumerate(seenIds)) {
    if (it.value())
      positions.insert((unsigned)it.index());
  }
  return positions;
}

template <typename T>
SmallVector<GridAxesAttr>
fromArrayOfVector(MLIRContext *ctxt, const SmallVector<SmallVector<T>> &vec) {
  SmallVector<GridAxesAttr> res;
  for (const auto &v : vec) {
    res.emplace_back(GridAxesAttr::get(ctxt, v));
  }
  return res;
}

//===----------------------------------------------------------------------===//
// shard::getSharding
//===----------------------------------------------------------------------===//

FailureOr<std::pair<bool, Sharding>> shard::getSharding(OpResult result) {
  Value val = cast<Value>(result);
  bool anyShardedForDef = toolchain::any_of(val.getUsers(), [](Operation *user) {
    auto shardOp = toolchain::dyn_cast<shard::ShardOp>(user);
    if (!shardOp)
      return false;
    return !shardOp.getAnnotateForUsers();
  });

  if (anyShardedForDef) {
    // expected to have exact one use if it has a use of `shard.shard` without
    // unit attr annotate_for_users
    if (!val.hasOneUse())
      return failure();
    auto shardOp = toolchain::cast<shard::ShardOp>(*val.getUsers().begin());
    return std::make_pair(false, Sharding(shardOp.getSharding()));
  }

  bool anyShardedForUsers = toolchain::any_of(val.getUsers(), [](Operation *user) {
    auto shardOp = toolchain::dyn_cast<shard::ShardOp>(user);
    if (!shardOp)
      return false;
    return shardOp.getAnnotateForUsers();
  });
  if (anyShardedForUsers) {
    SmallVector<ShardOp> shardOps;
    for (Operation *user : val.getUsers()) {
      ShardOp shardOp = toolchain::dyn_cast<ShardOp>(user);
      if (shardOp)
        shardOps.push_back(shardOp);
    }
    Sharding shardForDef = shardOps[0].getSharding();
    for (size_t i = 1; i < shardOps.size(); ++i) {
      // TODO: Deduce a reasonable grid sharding attr for def when they are
      // different
      assert(shardForDef == shardOps[i].getSharding() &&
             "only support all shard ops have the same grid sharding attr");
    }
    return std::make_pair(true, shardForDef);
  }
  return failure();
}

FailureOr<std::pair<bool, Sharding>> shard::getSharding(OpOperand &opOperand) {
  Value val = opOperand.get();
  if (ShardOp shardOp = val.getDefiningOp<ShardOp>())
    return std::make_pair(shardOp.getAnnotateForUsers(),
                          Sharding(shardOp.getSharding()));

  return failure();
}

//===----------------------------------------------------------------------===//
// ShardingInterface::verifyShardingInterfaceImpl
//===----------------------------------------------------------------------===//

LogicalResult shard::ShardingInterface::verifyShardingInterfaceImpl() {
  Operation *op = getOperation();

  // check operands and results type
  for (Type type : op->getOperandTypes())
    if (!toolchain::isa<RankedTensorType>(type) && !type.isIntOrIndexOrFloat())
      return failure();
  for (Type type : op->getResultTypes())
    if (!toolchain::isa<RankedTensorType>(type) && !type.isIntOrIndexOrFloat())
      return failure();

  // check maps
  SmallVector<AffineMap> maps = getIndexingMaps();
  if (maps.empty())
    return failure();
  unsigned numOperands = op->getNumOperands();
  unsigned numResults = op->getNumResults();
  if (numOperands + numResults != maps.size())
    return failure();

  for (OpResult result : op->getResults()) {
    auto resultType = dyn_cast<RankedTensorType>(result.getType());
    if (!resultType)
      return failure();
    AffineMap map = maps[numOperands + result.getResultNumber()];
    if (!map.isProjectedPermutation()) {
      return failure();
    }
  }

  return success();
}

//===----------------------------------------------------------------------===//
// ShardingInterface::printLoopTypesAndIndexingMaps
//===----------------------------------------------------------------------===//

void shard::ShardingInterface::printLoopTypesAndIndexingMaps(raw_ostream &os) {
  os << "print loop types and indexing maps for: \n";
  getOperation()->print(os);
  os << "\n";
  os << "loop types: [";
  for (utils::IteratorType type : getLoopIteratorTypes()) {
    os << stringifyEnum(type) << " ";
  }
  os << "]\n";
  os << "indexing maps: \n";
  for (AffineMap map : getIndexingMaps())
    os << map << "\n";
  os << "\n";
}

//===----------------------------------------------------------------------===//
// detail::defaultGetShardingOption
//===----------------------------------------------------------------------===//

namespace {

// Update the given `shardingOption` according to `gridAxes` and `loopIdx`
static LogicalResult fillShardingOption(Operation *op,
                                        ShardingOption &shardingOption,
                                        FlatSymbolRefAttr grid,
                                        ArrayRef<GridAxis> gridAxes,
                                        unsigned loopIdx) {
  if ((shardingOption.grid && grid && shardingOption.grid != grid) ||
      (!shardingOption.shardingArray[loopIdx].empty() &&
       shardingOption.shardingArray[loopIdx] != gridAxes)) {
    LLVM_DEBUG(DBGS() << "sharding option conflicts on loop iterator "
                      << loopIdx << "\n");
    return failure();
  }
  for (size_t i = 0; i < shardingOption.shardingArray.size(); ++i) {
    if (i == loopIdx)
      continue;

    for (GridAxis axis : gridAxes) {
      if (toolchain::is_contained(shardingOption.shardingArray[i], axis)) {
        LLVM_DEBUG(DBGS() << "sharding option conflicts because grid axes "
                          << axis << " duplicate");
        return failure();
      }
    }
  }
  if (grid)
    shardingOption.grid = grid;
  if (shardingOption.shardingArray[loopIdx].empty())
    shardingOption.shardingArray[loopIdx].append(gridAxes.begin(),
                                                 gridAxes.end());
  return success();
}

} // namespace

FailureOr<ShardingOption>
shard::detail::defaultGetShardingOption(Operation *op,
                                        ArrayRef<Sharding> operandShardings,
                                        ArrayRef<Sharding> resultShardings) {
  ShardingInterface shardingOp = toolchain::cast<ShardingInterface>(op);
  ShardingOption shardingOption;

  if (failed(shardingOp.verifyShardingInterfaceImpl()))
    return op->emitOpError() << "invalid sharding interface implementation";
  SmallVector<utils::IteratorType> loopTypes =
      shardingOp.getLoopIteratorTypes();
  SmallVector<AffineMap> maps = shardingOp.getIndexingMaps();
  unsigned numOperands = op->getNumOperands();
  shardingOption.shardingArray.resize(loopTypes.size());
  toolchain::SmallSet<unsigned, 4> visitedLoopIndices;
  bool anyShardingInResultsOrOperands = false;

  // 1. Fill sharding option based on op results
  for (auto shardingIt : toolchain::enumerate(resultShardings)) {
    const Sharding &shardAttr = shardingIt.value();
    if (!shardAttr)
      continue;
    AffineMap map = maps[numOperands + shardingIt.index()];
    anyShardingInResultsOrOperands = true;
    if (shardAttr.getSplitAxes().empty() || map.getResults().empty()) {
      shardingOption.grid = shardAttr.getGridAttr();
    } else {
      // Handle the split axes: calculate the corresponding loop index for each
      // split axes sub-array, and then store the sub-array to
      // shardingOption[index]
      for (auto it : toolchain::zip(map.getResults(), shardAttr.getSplitAxes())) {
        AffineExpr expr = std::get<0>(it);
        ArrayRef<GridAxis> axes = std::get<1>(it).asArrayRef();
        auto dim = cast<AffineDimExpr>(expr);
        unsigned index = dim.getPosition();
        visitedLoopIndices.insert(index);
        if (failed(fillShardingOption(op, shardingOption,
                                      shardAttr.getGridAttr(), axes, index)))
          return failure();
      }
    }
  }

  // 2. Fill sharding option based on operands
  for (auto shardingIt : toolchain::enumerate(operandShardings)) {
    const Sharding &shardAttr = shardingIt.value();
    if (!shardAttr)
      continue;

    anyShardingInResultsOrOperands = !shardAttr.getSplitAxes().empty();
    AffineMap map = maps[shardingIt.index()];
    unsigned numDims = map.getNumDims();

    // Handle the split axes.
    //
    // TODO: Change to process the operands with single loop index first and
    // then the operands with multiple loop indices.
    for (auto it : toolchain::zip(map.getResults(), shardAttr.getSplitAxes())) {
      AffineExpr expr = std::get<0>(it);
      ArrayRef<GridAxis> axes = std::get<1>(it).asArrayRef();
      FailureOr<toolchain::SmallSet<unsigned, 2>> loopIndices =
          checkOperandAffineExpr(expr, numDims);
      if (failed(loopIndices))
        return op->emitOpError()
               << "operand's affine expression is restricted to const_i * "
                  "dim_i + const_j + dim_j + ...";
      if (loopIndices->empty())
        continue;
      if (loopIndices->size() == 1) {
        unsigned loopIdx = *loopIndices->begin();
        visitedLoopIndices.insert(loopIdx);
        if (failed(fillShardingOption(op, shardingOption,
                                      shardAttr.getGridAttr(), axes, loopIdx)))
          return failure();
      }
      // If multiple loop indices correspond to a dimension of an operand, it is
      // difficult to infer which loop indices are responsible for sharding.
      // Therefore, the exact loop index must be specified by others.
      if (loopIndices->size() > 1) {
        bool seenLoopIndices = false;
        for (unsigned loopIdx : *loopIndices) {
          if (visitedLoopIndices.contains(loopIdx)) {
            seenLoopIndices = true;
            break;
          }
        }
        if (!seenLoopIndices)
          return op->emitOpError()
                 << "the operand " << shardingIt.index()
                 << " has multiple loop indices in a dimension, but none of "
                    "them could be found in the exactly specified annotation "
                    "of op results or operands.";
      }
    }
  }

  // 3. Finalize sharding option
  removeTrailingEmptySubArray(shardingOption.shardingArray);
  if (!anyShardingInResultsOrOperands)
    shardingOption.empty = true;
  return shardingOption;
}

// Get the sharding attributed for the given result and sharding option.
static Sharding getSharding(OpResult result,
                            const ShardingOption &shardingOption, AffineMap map,
                            ArrayRef<utils::IteratorType> loopTypes) {
  auto resultType = cast<RankedTensorType>(result.getType());
  SmallVector<SmallVector<GridAxis>> splitAxes(resultType.getRank());

  // process the split axes
  for (auto it : toolchain::enumerate(map.getResults())) {
    AffineExpr expr = it.value();
    // `expr` must be an `AffineDimExpr` because `map` is verified by
    // isProjectedPermutation
    auto dim = cast<AffineDimExpr>(expr);
    unsigned loopIdx = dim.getPosition();
    if (loopIdx < shardingOption.shardingArray.size())
      splitAxes[it.index()].append(shardingOption.shardingArray[loopIdx]);
  }

  removeTrailingEmptySubArray(splitAxes);
  return Sharding::get(shardingOption.grid,
                       fromArrayOfVector(result.getContext(), splitAxes));
}

static FailureOr<Sharding> getSharding(OpOperand &opOperand,
                                       const ShardingOption &shardingOption,
                                       AffineMap map) {
  Value operandValue = opOperand.get();
  auto operandType = dyn_cast<RankedTensorType>(operandValue.getType());
  if (!operandType) {
    if (operandValue.getType().isIntOrIndexOrFloat())
      return Sharding();
    return failure();
  }
  // 0d tensors cannot be sharded and must get replicated
  if (operandType.getRank() == 0) {
    return Sharding(shardingOption.grid);
  }
  SmallVector<SmallVector<GridAxis>> splitAxes(operandType.getRank());
  unsigned numDims = map.getNumDims();
  for (auto it : toolchain::enumerate(map.getResults())) {
    int64_t idx = it.index();
    AffineExpr expr = it.value();
    FailureOr<toolchain::SmallSet<unsigned, 2>> loopIndices =
        checkOperandAffineExpr(expr, numDims);
    if (failed(loopIndices))
      return failure();
    SmallVector<unsigned> shardedLoopIndices;
    for (unsigned loopIdx : *loopIndices) {
      if ((size_t)loopIdx < shardingOption.shardingArray.size() &&
          !shardingOption.shardingArray[loopIdx].empty())
        shardedLoopIndices.push_back(loopIdx);
    }
    // mostly one sharded loop index is accepted
    if (shardedLoopIndices.size() > 1)
      return failure();
    if (shardedLoopIndices.size() == 1) {
      splitAxes[idx].append(
          shardingOption.shardingArray[shardedLoopIndices[0]]);
    }
  }

  removeTrailingEmptySubArray(splitAxes);
  return Sharding::get(
      shardingOption.grid,
      fromArrayOfVector(opOperand.get().getContext(), splitAxes));
}

FailureOr<std::vector<Sharding>> shard::detail::defaultGetShardingAnnotations(
    Operation *op, const ShardingOption &shardingOption) {
  std::vector<Sharding> res;

  ShardingInterface shardingOp = toolchain::cast<ShardingInterface>(op);
  SmallVector<utils::IteratorType> loopTypes =
      shardingOp.getLoopIteratorTypes();
  SmallVector<AffineMap> maps = shardingOp.getIndexingMaps();
  unsigned numOperands = op->getNumOperands();

  for (OpOperand &opOperand : op->getOpOperands()) {
    FailureOr<Sharding> shardingAttr = ::getSharding(
        opOperand, shardingOption, maps[opOperand.getOperandNumber()]);
    if (failed(shardingAttr))
      return failure();
    res.push_back(*shardingAttr);
  }

  for (OpResult result : op->getResults()) {
    res.push_back(::getSharding(result, shardingOption,
                                maps[numOperands + result.getResultNumber()],
                                loopTypes));
  }

  return res;
}

//===----------------------------------------------------------------------===//
// detail::defaultAddShardingAnnotations
//===----------------------------------------------------------------------===//

// To add a `shard.shard` op for the given result, based on the details provided
// in `shardingOption`, `map`, and `loopTypes`.
static LogicalResult addShardOp(OpBuilder &b, OpResult result,
                                const ShardingOption &shardingOption,
                                AffineMap map,
                                ArrayRef<utils::IteratorType> loopTypes) {
  Sharding sharding = getSharding(result, shardingOption, map, loopTypes);
  maybeInsertTargetShardingAnnotation(sharding, result, b);

  return success();
}

// To add a `shard.shard` op for the given operand, based on the details
// provided in `shardingOption`, `map`, and `loopTypes`.
static LogicalResult addShardOp(OpBuilder &b, OpOperand &opOperand,
                                const ShardingOption &shardingOption,
                                AffineMap map) {

  FailureOr<Sharding> sharding = getSharding(opOperand, shardingOption, map);
  if (failed(sharding)) {
    return failure();
  }
  OpBuilder::InsertionGuard guard(b);
  maybeInsertSourceShardingAnnotation(sharding.value(), opOperand, b);

  return success();
}

LogicalResult shard::detail::defaultAddShardingAnnotations(
    Operation *op, OpBuilder &b, const ShardingOption &shardingOption) {
  assert(!shardingOption.empty && shardingOption.grid);

  ShardingInterface shardingOp = toolchain::cast<ShardingInterface>(op);
  SmallVector<utils::IteratorType> loopTypes =
      shardingOp.getLoopIteratorTypes();
  SmallVector<AffineMap> maps = shardingOp.getIndexingMaps();
  unsigned numOperands = op->getNumOperands();

  // 1. add shard.shard ops for all op results
  for (OpResult result : op->getResults()) {
    if (failed(addShardOp(b, result, shardingOption,
                          maps[numOperands + result.getResultNumber()],
                          loopTypes)))
      return failure();
  }

  // 2. add shard.shard ops for all operands
  for (OpOperand &opOperand : op->getOpOperands()) {
    if (failed(addShardOp(b, opOperand, shardingOption,
                          maps[opOperand.getOperandNumber()])))
      return failure();
  }

  return success();
}

#ifndef NDEBUG
static bool
isValueCompatibleWithFullReplicationSharding(Value value,
                                             const Sharding &sharding) {
  if (isa<RankedTensorType>(value.getType())) {
    return isFullReplication(sharding);
  }

  return !sharding;
}

template <typename ValueRange, typename ShardingRage>
static bool
areValuesCompatibleWithFullReplicationShardings(ValueRange &&values,
                                                ShardingRage &&shardings) {
  if (std::size(values) != std::size(shardings)) {
    return false;
  }
  return toolchain::all_of(toolchain::zip_equal(std::forward<ValueRange>(values),
                                      std::forward<ShardingRage>(shardings)),
                      [](auto valueAndSharding) {
                        return isValueCompatibleWithFullReplicationSharding(
                            std::get<0>(valueAndSharding),
                            std::get<1>(valueAndSharding));
                      });
}
#endif // NDEBUG

void shard::partitionFullyReplicatedOperation(
    Operation &op, ArrayRef<Value> partitionedOperands,
    ArrayRef<Sharding> operandShardings, ArrayRef<Sharding> resultShardings,
    IRMapping &partitionMap, SymbolTableCollection &symbolTable,
    OpBuilder &builder) {
  assert(partitionedOperands.size() == operandShardings.size());
  assert(areValuesCompatibleWithFullReplicationShardings(op.getOperands(),
                                                         operandShardings));
  assert(areValuesCompatibleWithFullReplicationShardings(op.getResults(),
                                                         resultShardings));
  // `clone` will populate the mapping of old to new results.
  builder.clone(op, partitionMap);
}

static void updateGridAxisAssignmentForLoopIterators(
    ArrayRef<GridAxis> gridAxesAssignmentForTensorAxis, AffineExpr indexingExpr,
    SmallVector<std::optional<SmallVector<GridAxis>>>
        &gridAxesAssignmentForLoopIterators) {
  AffineDimExpr affineDimExpr = cast<AffineDimExpr>(indexingExpr);
  unsigned loopIteratorIdx = affineDimExpr.getPosition();
  if (gridAxesAssignmentForLoopIterators[loopIteratorIdx]) {
    assert(toolchain::equal(gridAxesAssignmentForTensorAxis,
                       *gridAxesAssignmentForLoopIterators[loopIteratorIdx]));
  } else {
    gridAxesAssignmentForLoopIterators[loopIteratorIdx] =
        toolchain::to_vector(gridAxesAssignmentForTensorAxis);
  }
}

ShardingArray shard::getGridAxisAssignmentForLoopIterators(
    ArrayRef<Sharding> operandShardings, ArrayRef<Sharding> resultShardings,
    ArrayRef<utils::IteratorType> loopIteratorTypes,
    ArrayRef<AffineMap> indexingMaps) {
  SmallVector<std::optional<SmallVector<GridAxis>>>
      gridAxisAssignmentForLoopIterators(loopIteratorTypes.size());
  std::vector<Sharding> operatorAndResultShardings;
  operatorAndResultShardings.reserve(operandShardings.size() +
                                     resultShardings.size());
  toolchain::append_range(operatorAndResultShardings, operandShardings);
  for (auto [sharding, affineMap] :
       toolchain::zip_equal(operatorAndResultShardings, indexingMaps)) {
    if (!sharding) {
      continue;
    }
    for (auto [gridAxesAssignmentForTensorAxis, indexingExpr] :
         toolchain::zip(sharding.getSplitAxes(), affineMap.getResults())) {
      updateGridAxisAssignmentForLoopIterators(
          gridAxesAssignmentForTensorAxis.asArrayRef(), indexingExpr,
          gridAxisAssignmentForLoopIterators);
    }
    // Missing trailing split axes means replication on those tensor dimensions.
    for (unsigned i = sharding.getSplitAxes().size();
         i < affineMap.getNumResults(); ++i) {
      updateGridAxisAssignmentForLoopIterators(
          {}, affineMap.getResults()[i], gridAxisAssignmentForLoopIterators);
    }
  }

  ShardingArray res;
  toolchain::transform(gridAxisAssignmentForLoopIterators, std::back_inserter(res),
                  [](std::optional<SmallVector<GridAxis>> &axes) {
                    if (!axes) {
                      return SmallVector<GridAxis>();
                    };
                    return std::move(*axes);
                  });
  return res;
}

bool shard::isAtLeastOneReductionIteratorSharded(
    ArrayRef<utils::IteratorType> loopIteratorTypes,
    ArrayRef<SmallVector<GridAxis>> gridAxisAssignmentForLoopIterators) {
  for (auto [loopIteratorType, gridAxisAssignment] :
       toolchain::zip_equal(loopIteratorTypes, gridAxisAssignmentForLoopIterators)) {
    if (loopIteratorType == utils::IteratorType::reduction &&
        !gridAxisAssignment.empty()) {
      return true;
    }
  }
  return false;
}

SmallVector<GridAxis> shard::getReductionGridAxes(
    ArrayRef<utils::IteratorType> loopIteratorTypes,
    ArrayRef<SmallVector<GridAxis>> gridAxisAssignmentForLoopIterators) {
  SmallVector<GridAxis> gridAxes;
  for (auto [loopIteratorType, gridAxisAssignment] :
       toolchain::zip_equal(loopIteratorTypes, gridAxisAssignmentForLoopIterators)) {
    if (loopIteratorType == utils::IteratorType::reduction) {
      toolchain::append_range(gridAxes, gridAxisAssignment);
    }
  }
  return gridAxes;
}

void shard::partitionTriviallyShardableOperation(
    Operation &op, ArrayRef<Value> partitionedOperands,
    ArrayRef<Sharding> operandShardings, ArrayRef<Sharding> resultShardings,
    IRMapping &partitionMap, SymbolTableCollection &symbolTable,
    OpBuilder &builder) {
  // `clone` will populate the mapping of old to new results.
  Operation *newOp = builder.clone(op, partitionMap);
  // Set the result types to the sharded counterparts.
  for (auto [oldResult, newResult, sharding] :
       toolchain::zip_equal(op.getResults(), newOp->getResults(), resultShardings)) {
    newResult.setType(shardType(
        newResult.getType(),
        getGridOrNull(&op, sharding.getGridAttr(), symbolTable), sharding));
  }
}
