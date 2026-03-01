//===- MemRefMemorySlot.cpp - Memory Slot Interfaces ------------*- C++ -*-===//
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
// This file implements Mem2Reg-related interfaces for MemRef dialect
// operations.
//
//===----------------------------------------------------------------------===//

#include "mlir/Dialect/MemRef/IR/MemRefMemorySlot.h"
#include "mlir/Dialect/MemRef/IR/MemRef.h"
#include "mlir/Dialect/UB/IR/UBOps.h"
#include "mlir/IR/BuiltinDialect.h"
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/Matchers.h"
#include "mlir/IR/Value.h"
#include "mlir/Interfaces/MemorySlotInterfaces.h"
#include "vm/core/ADT/ArrayRef.h"
#include "vm/core/ADT/TypeSwitch.h"
#include "vm/core/Support/ErrorHandling.h"

using namespace mlir;

//===----------------------------------------------------------------------===//
//  Utilities
//===----------------------------------------------------------------------===//

/// Walks over the indices of the elements of a tensor of a given `shape` by
/// updating `index` in place to the next index. This returns failure if the
/// provided index was the last index.
static LogicalResult nextIndex(ArrayRef<int64_t> shape,
                               MutableArrayRef<int64_t> index) {
  for (size_t i = 0; i < shape.size(); ++i) {
    index[i]++;
    if (index[i] < shape[i])
      return success();
    index[i] = 0;
  }
  return failure();
}

/// Calls `walker` for each index within a tensor of a given `shape`, providing
/// the index as an array attribute of the coordinates.
template <typename CallableT>
static void walkIndicesAsAttr(MLIRContext *ctx, ArrayRef<int64_t> shape,
                              CallableT &&walker) {
  Type indexType = IndexType::get(ctx);
  SmallVector<int64_t> shapeIter(shape.size(), 0);
  do {
    SmallVector<Attribute> indexAsAttr;
    for (int64_t dim : shapeIter)
      indexAsAttr.push_back(IntegerAttr::get(indexType, dim));
    walker(ArrayAttr::get(ctx, indexAsAttr));
  } while (succeeded(nextIndex(shape, shapeIter)));
}

//===----------------------------------------------------------------------===//
//  Interfaces for AllocaOp
//===----------------------------------------------------------------------===//

SmallVector<MemorySlot> memref::AllocaOp::getPromotableSlots() {
  MemRefType type = getType();
  if (!type.hasStaticShape())
    return {};
  // Make sure the memref contains only a single element.
  if (type.getNumElements() != 1)
    return {};

  return {MemorySlot{getResult(), type.getElementType()}};
}

Value memref::AllocaOp::getDefaultValue(const MemorySlot &slot,
                                        OpBuilder &builder) {
  return ub::PoisonOp::create(builder, getLoc(), slot.elemType);
}

std::optional<PromotableAllocationOpInterface>
memref::AllocaOp::handlePromotionComplete(const MemorySlot &slot,
                                          Value defaultValue,
                                          OpBuilder &builder) {
  if (defaultValue.use_empty())
    defaultValue.getDefiningOp()->erase();
  this->erase();
  return std::nullopt;
}

void memref::AllocaOp::handleBlockArgument(const MemorySlot &slot,
                                           BlockArgument argument,
                                           OpBuilder &builder) {}

SmallVector<DestructurableMemorySlot>
memref::AllocaOp::getDestructurableSlots() {
  MemRefType memrefType = getType();
  auto destructurable = toolchain::dyn_cast<DestructurableTypeInterface>(memrefType);
  if (!destructurable)
    return {};

  std::optional<DenseMap<Attribute, Type>> destructuredType =
      destructurable.getSubelementIndexMap();
  if (!destructuredType)
    return {};

  return {
      DestructurableMemorySlot{{getMemref(), memrefType}, *destructuredType}};
}

DenseMap<Attribute, MemorySlot> memref::AllocaOp::destructure(
    const DestructurableMemorySlot &slot,
    const SmallPtrSetImpl<Attribute> &usedIndices, OpBuilder &builder,
    SmallVectorImpl<DestructurableAllocationOpInterface> &newAllocators) {
  builder.setInsertionPointAfter(*this);

  DenseMap<Attribute, MemorySlot> slotMap;

  auto memrefType = toolchain::cast<DestructurableTypeInterface>(getType());
  for (Attribute usedIndex : usedIndices) {
    Type elemType = memrefType.getTypeAtIndex(usedIndex);
    MemRefType elemPtr = MemRefType::get({}, elemType);
    auto subAlloca = memref::AllocaOp::create(builder, getLoc(), elemPtr);
    newAllocators.push_back(subAlloca);
    slotMap.try_emplace<MemorySlot>(usedIndex,
                                    {subAlloca.getResult(), elemType});
  }

  return slotMap;
}

std::optional<DestructurableAllocationOpInterface>
memref::AllocaOp::handleDestructuringComplete(
    const DestructurableMemorySlot &slot, OpBuilder &builder) {
  assert(slot.ptr == getResult());
  this->erase();
  return std::nullopt;
}

//===----------------------------------------------------------------------===//
//  Interfaces for LoadOp/StoreOp
//===----------------------------------------------------------------------===//

bool memref::LoadOp::loadsFrom(const MemorySlot &slot) {
  return getMemRef() == slot.ptr;
}

bool memref::LoadOp::storesTo(const MemorySlot &slot) { return false; }

Value memref::LoadOp::getStored(const MemorySlot &slot, OpBuilder &builder,
                                Value reachingDef,
                                const DataLayout &dataLayout) {
  llvm_unreachable("getStored should not be called on LoadOp");
}

bool memref::LoadOp::canUsesBeRemoved(
    const MemorySlot &slot, const SmallPtrSetImpl<OpOperand *> &blockingUses,
    SmallVectorImpl<OpOperand *> &newBlockingUses,
    const DataLayout &dataLayout) {
  if (blockingUses.size() != 1)
    return false;
  Value blockingUse = (*blockingUses.begin())->get();
  return blockingUse == slot.ptr && getMemRef() == slot.ptr &&
         getResult().getType() == slot.elemType;
}

DeletionKind memref::LoadOp::removeBlockingUses(
    const MemorySlot &slot, const SmallPtrSetImpl<OpOperand *> &blockingUses,
    OpBuilder &builder, Value reachingDefinition,
    const DataLayout &dataLayout) {
  // `canUsesBeRemoved` checked this blocking use must be the loaded slot
  // pointer.
  getResult().replaceAllUsesWith(reachingDefinition);
  return DeletionKind::Delete;
}

/// Returns the index of a memref in attribute form, given its indices. Returns
/// a null pointer if whether the indices form a valid index for the provided
/// MemRefType cannot be computed. The indices must come from a valid memref
/// StoreOp or LoadOp.
static Attribute getAttributeIndexFromIndexOperands(MLIRContext *ctx,
                                                    ValueRange indices,
                                                    MemRefType memrefType) {
  SmallVector<Attribute> index;
  for (auto [coord, dimSize] : toolchain::zip(indices, memrefType.getShape())) {
    IntegerAttr coordAttr;
    if (!matchPattern(coord, m_Constant<IntegerAttr>(&coordAttr)))
      return {};
    // MemRefType shape dimensions are always positive (checked by verifier).
    std::optional<uint64_t> coordInt = coordAttr.getValue().tryZExtValue();
    if (!coordInt || coordInt.value() >= static_cast<uint64_t>(dimSize))
      return {};
    index.push_back(coordAttr);
  }
  return ArrayAttr::get(ctx, index);
}

bool memref::LoadOp::canRewire(const DestructurableMemorySlot &slot,
                               SmallPtrSetImpl<Attribute> &usedIndices,
                               SmallVectorImpl<MemorySlot> &mustBeSafelyUsed,
                               const DataLayout &dataLayout) {
  if (slot.ptr != getMemRef())
    return false;
  Attribute index = getAttributeIndexFromIndexOperands(
      getContext(), getIndices(), getMemRefType());
  if (!index)
    return false;
  usedIndices.insert(index);
  return true;
}

DeletionKind memref::LoadOp::rewire(const DestructurableMemorySlot &slot,
                                    DenseMap<Attribute, MemorySlot> &subslots,
                                    OpBuilder &builder,
                                    const DataLayout &dataLayout) {
  Attribute index = getAttributeIndexFromIndexOperands(
      getContext(), getIndices(), getMemRefType());
  const MemorySlot &memorySlot = subslots.at(index);
  setMemRef(memorySlot.ptr);
  getIndicesMutable().clear();
  return DeletionKind::Keep;
}

bool memref::StoreOp::loadsFrom(const MemorySlot &slot) { return false; }

bool memref::StoreOp::storesTo(const MemorySlot &slot) {
  return getMemRef() == slot.ptr;
}

Value memref::StoreOp::getStored(const MemorySlot &slot, OpBuilder &builder,
                                 Value reachingDef,
                                 const DataLayout &dataLayout) {
  return getValue();
}

bool memref::StoreOp::canUsesBeRemoved(
    const MemorySlot &slot, const SmallPtrSetImpl<OpOperand *> &blockingUses,
    SmallVectorImpl<OpOperand *> &newBlockingUses,
    const DataLayout &dataLayout) {
  if (blockingUses.size() != 1)
    return false;
  Value blockingUse = (*blockingUses.begin())->get();
  return blockingUse == slot.ptr && getMemRef() == slot.ptr &&
         getValue() != slot.ptr && getValue().getType() == slot.elemType;
}

DeletionKind memref::StoreOp::removeBlockingUses(
    const MemorySlot &slot, const SmallPtrSetImpl<OpOperand *> &blockingUses,
    OpBuilder &builder, Value reachingDefinition,
    const DataLayout &dataLayout) {
  return DeletionKind::Delete;
}

bool memref::StoreOp::canRewire(const DestructurableMemorySlot &slot,
                                SmallPtrSetImpl<Attribute> &usedIndices,
                                SmallVectorImpl<MemorySlot> &mustBeSafelyUsed,
                                const DataLayout &dataLayout) {
  if (slot.ptr != getMemRef() || getValue() == slot.ptr)
    return false;
  Attribute index = getAttributeIndexFromIndexOperands(
      getContext(), getIndices(), getMemRefType());
  if (!index || !slot.subelementTypes.contains(index))
    return false;
  usedIndices.insert(index);
  return true;
}

DeletionKind memref::StoreOp::rewire(const DestructurableMemorySlot &slot,
                                     DenseMap<Attribute, MemorySlot> &subslots,
                                     OpBuilder &builder,
                                     const DataLayout &dataLayout) {
  Attribute index = getAttributeIndexFromIndexOperands(
      getContext(), getIndices(), getMemRefType());
  const MemorySlot &memorySlot = subslots.at(index);
  setMemRef(memorySlot.ptr);
  getIndicesMutable().clear();
  return DeletionKind::Keep;
}

//===----------------------------------------------------------------------===//
//  Interfaces for destructurable types
//===----------------------------------------------------------------------===//

namespace {

struct MemRefDestructurableTypeExternalModel
    : public DestructurableTypeInterface::ExternalModel<
          MemRefDestructurableTypeExternalModel, MemRefType> {
  std::optional<DenseMap<Attribute, Type>>
  getSubelementIndexMap(Type type) const {
    auto memrefType = toolchain::cast<MemRefType>(type);
    constexpr int64_t maxMemrefSizeForDestructuring = 16;
    if (!memrefType.hasStaticShape() ||
        memrefType.getNumElements() > maxMemrefSizeForDestructuring ||
        memrefType.getNumElements() == 1)
      return {};

    DenseMap<Attribute, Type> destructured;
    walkIndicesAsAttr(
        memrefType.getContext(), memrefType.getShape(), [&](Attribute index) {
          destructured.insert({index, memrefType.getElementType()});
        });

    return destructured;
  }

  Type getTypeAtIndex(Type type, Attribute index) const {
    auto memrefType = toolchain::cast<MemRefType>(type);
    auto coordArrAttr = toolchain::dyn_cast<ArrayAttr>(index);
    if (!coordArrAttr || coordArrAttr.size() != memrefType.getShape().size())
      return {};

    Type indexType = IndexType::get(memrefType.getContext());
    for (const auto &[coordAttr, dimSize] :
         toolchain::zip(coordArrAttr, memrefType.getShape())) {
      auto coord = toolchain::dyn_cast<IntegerAttr>(coordAttr);
      if (!coord || coord.getType() != indexType || coord.getInt() < 0 ||
          coord.getInt() >= dimSize)
        return {};
    }

    return memrefType.getElementType();
  }
};

} // namespace

//===----------------------------------------------------------------------===//
//  Register external models
//===----------------------------------------------------------------------===//

void mlir::memref::registerMemorySlotExternalModels(DialectRegistry &registry) {
  registry.addExtension(+[](MLIRContext *ctx, BuiltinDialect *dialect) {
    MemRefType::attachInterface<MemRefDestructurableTypeExternalModel>(*ctx);
  });
}
