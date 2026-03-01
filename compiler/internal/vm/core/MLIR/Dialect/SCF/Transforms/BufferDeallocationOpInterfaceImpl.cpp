//===- BufferDeallocationOpInterfaceImpl.cpp ------------------------------===//
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

#include "mlir/Dialect/SCF/Transforms/BufferDeallocationOpInterfaceImpl.h"
#include "mlir/Dialect/Bufferization/IR/BufferDeallocationOpInterface.h"
#include "mlir/Dialect/SCF/IR/SCF.h"

using namespace mlir;
using namespace mlir::bufferization;

namespace {
/// The `scf.forall.in_parallel` terminator is special in a few ways:
/// * It does not implement the BranchOpInterface or
///   RegionBranchTerminatorOpInterface, but the InParallelOpInterface
///   which is not supported by BufferDeallocation.
/// * It has a graph-like region which only allows one specific tensor op
/// * After bufferization the nested region is always empty
/// For these reasons we provide custom deallocation logic via this external
/// model.
///
/// Example:
/// ```mlir
/// scf.forall (%arg1) in (%arg0) {
///   %alloc = memref.alloc() : memref<2xf32>
///   ...
///   <implicit in_parallel terminator here>
/// }
/// ```
/// gets transformed to
/// ```mlir
/// scf.forall (%arg1) in (%arg0) {
///   %alloc = memref.alloc() : memref<2xf32>
///   ...
///   bufferization.dealloc (%alloc : memref<2xf32>) if (%true)
///   <implicit in_parallel terminator here>
/// }
/// ```
struct InParallelDeallocOpInterface
    : public BufferDeallocationOpInterface::ExternalModel<
          InParallelDeallocOpInterface, scf::InParallelOp> {
  FailureOr<Operation *> process(Operation *op, DeallocationState &state,
                                 const DeallocationOptions &options) const {
    auto inParallelOp = cast<scf::InParallelOp>(op);
    if (!inParallelOp.getBody()->empty())
      return op->emitError("only supported when nested region is empty");

    SmallVector<Value> updatedOperandOwnership;
    return deallocation_impl::insertDeallocOpForReturnLike(
        state, op, {}, updatedOperandOwnership);
  }
};

struct ReduceReturnOpInterface
    : public BufferDeallocationOpInterface::ExternalModel<
          ReduceReturnOpInterface, scf::ReduceReturnOp> {
  FailureOr<Operation *> process(Operation *op, DeallocationState &state,
                                 const DeallocationOptions &options) const {
    auto reduceReturnOp = cast<scf::ReduceReturnOp>(op);
    if (isa<BaseMemRefType>(reduceReturnOp.getOperand().getType()))
      return op->emitError("only supported when operand is not a MemRef");

    SmallVector<Value> updatedOperandOwnership;
    return deallocation_impl::insertDeallocOpForReturnLike(
        state, op, {}, updatedOperandOwnership);
  }
};

} // namespace

void mlir::scf::registerBufferDeallocationOpInterfaceExternalModels(
    DialectRegistry &registry) {
  registry.addExtension(+[](MLIRContext *ctx, SCFDialect *dialect) {
    InParallelOp::attachInterface<InParallelDeallocOpInterface>(*ctx);
    ReduceReturnOp::attachInterface<ReduceReturnOpInterface>(*ctx);
  });
}
