//===- AllocationOpInterfaceImpl.cpp - Impl. of AllocationOpInterface -----===//
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

#include "mlir/Dialect/MemRef/Transforms/AllocationOpInterfaceImpl.h"

#include "mlir/Dialect/Bufferization/IR/AllocationOpInterface.h"
#include "mlir/Dialect/Bufferization/IR/Bufferization.h"
#include "mlir/Dialect/MemRef/IR/MemRef.h"
#include "mlir/IR/Dialect.h"
#include "mlir/IR/Operation.h"

using namespace mlir;

namespace {
struct DefaultAllocationInterface
    : public bufferization::AllocationOpInterface::ExternalModel<
          DefaultAllocationInterface, memref::AllocOp> {
  static std::optional<Operation *> buildDealloc(OpBuilder &builder,
                                                 Value alloc) {
    return memref::DeallocOp::create(builder, alloc.getLoc(), alloc)
        .getOperation();
  }
  static std::optional<Value> buildClone(OpBuilder &builder, Value alloc) {
    return bufferization::CloneOp::create(builder, alloc.getLoc(), alloc)
        .getResult();
  }
  static ::mlir::HoistingKind getHoistingKind() {
    return HoistingKind::Loop | HoistingKind::Block;
  }
  static ::std::optional<::mlir::Operation *>
  buildPromotedAlloc(OpBuilder &builder, Value alloc) {
    Operation *definingOp = alloc.getDefiningOp();
    return memref::AllocaOp::create(
        builder, definingOp->getLoc(),
        cast<MemRefType>(definingOp->getResultTypes()[0]),
        definingOp->getOperands(), definingOp->getAttrs());
  }
};

struct DefaultAutomaticAllocationHoistingInterface
    : public bufferization::AllocationOpInterface::ExternalModel<
          DefaultAutomaticAllocationHoistingInterface, memref::AllocaOp> {
  static ::mlir::HoistingKind getHoistingKind() { return HoistingKind::Loop; }
};

struct DefaultReallocationInterface
    : public bufferization::AllocationOpInterface::ExternalModel<
          DefaultAllocationInterface, memref::ReallocOp> {
  static std::optional<Operation *> buildDealloc(OpBuilder &builder,
                                                 Value realloc) {
    return memref::DeallocOp::create(builder, realloc.getLoc(), realloc)
        .getOperation();
  }
};
} // namespace

void mlir::memref::registerAllocationOpInterfaceExternalModels(
    DialectRegistry &registry) {
  registry.addExtension(+[](MLIRContext *ctx, memref::MemRefDialect *dialect) {
    memref::AllocOp::attachInterface<DefaultAllocationInterface>(*ctx);
    memref::AllocaOp::attachInterface<
        DefaultAutomaticAllocationHoistingInterface>(*ctx);
    memref::ReallocOp::attachInterface<DefaultReallocationInterface>(*ctx);
  });
}
