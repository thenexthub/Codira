//===----------------------------------------------------------------------===//
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

#include "mlir/Conversion/ConvertToEmitC/ToEmitCInterface.h"
#include "mlir/Conversion/ConvertToLLVM/ToLLVMInterface.h"
#include "mlir/Dialect/Bufferization/IR/AllocationOpInterface.h"
#include "mlir/Dialect/MemRef/IR/MemRef.h"
#include "mlir/Dialect/UB/IR/UBOps.h"
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/Interfaces/MemorySlotInterfaces.h"
#include "mlir/Interfaces/RuntimeVerifiableOpInterface.h"
#include "mlir/Interfaces/SideEffectInterfaces.h"
#include "mlir/Interfaces/ValueBoundsOpInterface.h"
#include "mlir/Transforms/InliningUtils.h"
#include <optional>

using namespace mlir;
using namespace mlir::memref;

#include "mlir/Dialect/MemRef/IR/MemRefOpsDialect.cpp.inc"

//===----------------------------------------------------------------------===//
// MemRefDialect Dialect Interfaces
//===----------------------------------------------------------------------===//

namespace {
struct MemRefInlinerInterface : public DialectInlinerInterface {
  using DialectInlinerInterface::DialectInlinerInterface;
  bool isLegalToInline(Region *dest, Region *src, bool wouldBeCloned,
                       IRMapping &valueMapping) const final {
    return true;
  }
  bool isLegalToInline(Operation *, Region *, bool wouldBeCloned,
                       IRMapping &) const final {
    return true;
  }
};
} // namespace

void mlir::memref::MemRefDialect::initialize() {
  addOperations<
#define GET_OP_LIST
#include "mlir/Dialect/MemRef/IR/MemRefOps.cpp.inc"
      >();
  addInterfaces<MemRefInlinerInterface>();
  declarePromisedInterface<ConvertToEmitCPatternInterface, MemRefDialect>();
  declarePromisedInterface<ConvertToLLVMPatternInterface, MemRefDialect>();
  declarePromisedInterfaces<bufferization::AllocationOpInterface, AllocOp,
                            AllocaOp, ReallocOp>();
  declarePromisedInterfaces<RuntimeVerifiableOpInterface, AssumeAlignmentOp,
                            AtomicRMWOp, CastOp, CopyOp, DimOp, ExpandShapeOp,
                            GenericAtomicRMWOp, LoadOp, StoreOp, SubViewOp>();
  declarePromisedInterfaces<ValueBoundsOpInterface, AllocOp, AllocaOp, CastOp,
                            DimOp, GetGlobalOp, RankOp, SubViewOp>();
  declarePromisedInterface<DestructurableTypeInterface, MemRefType>();
}

/// Finds the unique dealloc operation (if one exists) for `allocValue`.
std::optional<Operation *> mlir::memref::findDealloc(Value allocValue) {
  Operation *dealloc = nullptr;
  for (Operation *user : allocValue.getUsers()) {
    if (!hasEffect<MemoryEffects::Free>(user, allocValue))
      continue;
    // If we found a realloc instead of dealloc, return std::nullopt.
    if (isa<memref::ReallocOp>(user))
      return std::nullopt;
    // If we found > 1 dealloc, return std::nullopt.
    if (dealloc)
      return std::nullopt;
    dealloc = user;
  }
  return dealloc;
}
