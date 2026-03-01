//===- MemOpInterfaces.cpp - Memory operation interfaces ---------*- C++-*-===//
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

#include "mlir/Interfaces/MemOpInterfaces.h"
#include "mlir/IR/Attributes.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/Value.h"

using namespace mlir;

LogicalResult mlir::detail::verifyMemorySpaceCastOpInterface(Operation *op) {
  auto memCastOp = cast<MemorySpaceCastOpInterface>(op);

  // Verify that the source and target pointers are valid
  Value sourcePtr = memCastOp.getSourcePtr();
  Value targetPtr = memCastOp.getTargetPtr();

  if (!sourcePtr || !targetPtr) {
    return op->emitError()
           << "memory space cast op must have valid source and target pointers";
  }

  if (sourcePtr.getType().getTypeID() != targetPtr.getType().getTypeID()) {
    return op->emitError()
           << "expected source and target types of the same kind";
  }

  // Verify the Types are of `PtrLikeTypeInterface` type.
  auto sourceType = dyn_cast<PtrLikeTypeInterface>(sourcePtr.getType());
  if (!sourceType) {
    return op->emitError()
           << "source type must implement `PtrLikeTypeInterface`, but got: "
           << sourcePtr.getType();
  }

  auto targetType = dyn_cast<PtrLikeTypeInterface>(targetPtr.getType());
  if (!targetType) {
    return op->emitError()
           << "target type must implement `PtrLikeTypeInterface`, but got: "
           << targetPtr.getType();
  }

  // Verify that the operation has exactly one result
  if (op->getNumResults() != 1) {
    return op->emitError()
           << "memory space cast op must have exactly one result";
  }

  return success();
}

FailureOr<std::optional<SmallVector<Value>>>
mlir::detail::bubbleDownInPlaceMemorySpaceCastImpl(OpOperand &operand,
                                                   ValueRange results) {
  MemorySpaceCastOpInterface castOp =
      MemorySpaceCastOpInterface::getIfPromotableCast(operand.get());

  // Bail if the src is not valid.
  if (!castOp)
    return failure();

  // Modify the op.
  operand.set(castOp.getSourcePtr());
  return std::optional<SmallVector<Value>>();
}

#include "mlir/Interfaces/MemOpInterfaces.cpp.inc"
