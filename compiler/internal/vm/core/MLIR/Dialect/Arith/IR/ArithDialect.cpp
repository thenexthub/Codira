//===- ArithDialect.cpp - MLIR Arith dialect implementation -----===//
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
#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Bufferization/IR/BufferDeallocationOpInterface.h"
#include "mlir/Dialect/Bufferization/IR/BufferizableOpInterface.h"
#include "mlir/Dialect/UB/IR/UBOps.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/DialectImplementation.h"
#include "mlir/Interfaces/ValueBoundsOpInterface.h"
#include "mlir/Transforms/InliningUtils.h"
#include "vm/core/ADT/TypeSwitch.h"

using namespace mlir;
using namespace mlir::arith;

#include "mlir/Dialect/Arith/IR/ArithOpsDialect.cpp.inc"
#include "mlir/Dialect/Arith/IR/ArithOpsInterfaces.cpp.inc"
#define GET_ATTRDEF_CLASSES
#include "mlir/Dialect/Arith/IR/ArithOpsAttributes.cpp.inc"

namespace {
/// This class defines the interface for handling inlining for arithmetic
/// dialect operations.
struct ArithInlinerInterface : public DialectInlinerInterface {
  using DialectInlinerInterface::DialectInlinerInterface;

  /// All arithmetic dialect ops can be inlined.
  bool isLegalToInline(Operation *, Region *, bool, IRMapping &) const final {
    return true;
  }
};
} // namespace

void arith::ArithDialect::initialize() {
  addOperations<
#define GET_OP_LIST
#include "mlir/Dialect/Arith/IR/ArithOps.cpp.inc"
      >();
  addAttributes<
#define GET_ATTRDEF_LIST
#include "mlir/Dialect/Arith/IR/ArithOpsAttributes.cpp.inc"
      >();
  addInterfaces<ArithInlinerInterface>();
  declarePromisedInterface<ConvertToEmitCPatternInterface, ArithDialect>();
  declarePromisedInterface<ConvertToLLVMPatternInterface, ArithDialect>();
  declarePromisedInterface<bufferization::BufferDeallocationOpInterface,
                           SelectOp>();
  declarePromisedInterfaces<bufferization::BufferizableOpInterface, ConstantOp,
                            IndexCastOp, SelectOp>();
  declarePromisedInterfaces<ValueBoundsOpInterface, AddIOp, ConstantOp, SubIOp,
                            MulIOp>();
}

/// Materialize an integer or floating point constant.
Operation *arith::ArithDialect::materializeConstant(OpBuilder &builder,
                                                    Attribute value, Type type,
                                                    Location loc) {
  if (auto poison = dyn_cast<ub::PoisonAttr>(value))
    return ub::PoisonOp::create(builder, loc, type, poison);

  return ConstantOp::materialize(builder, value, type, loc);
}
