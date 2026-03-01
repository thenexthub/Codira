//===- UBOps.cpp - UB Dialect Operations ----------------------------------===//
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

#include "mlir/Dialect/UB/IR/UBOps.h"
#include "mlir/Conversion/ConvertToLLVM/ToLLVMInterface.h"
#include "mlir/Transforms/InliningUtils.h"

#include "mlir/IR/Builders.h"
#include "mlir/IR/DialectImplementation.h"
#include "vm/core/ADT/TypeSwitch.h"

#include "mlir/Dialect/UB/IR/UBOpsDialect.cpp.inc"

using namespace mlir;
using namespace mlir::ub;

namespace {
/// This class defines the interface for handling inlining with UB
/// operations.
struct UBInlinerInterface : public DialectInlinerInterface {
  using DialectInlinerInterface::DialectInlinerInterface;

  /// All UB ops can be inlined.
  bool isLegalToInline(Operation *, Region *, bool, IRMapping &) const final {
    return true;
  }
};
} // namespace

//===----------------------------------------------------------------------===//
// UBDialect
//===----------------------------------------------------------------------===//

void UBDialect::initialize() {
  addOperations<
#define GET_OP_LIST
#include "mlir/Dialect/UB/IR/UBOps.cpp.inc"
      >();
  addAttributes<
#define GET_ATTRDEF_LIST
#include "mlir/Dialect/UB/IR/UBOpsAttributes.cpp.inc"
      >();
  addInterfaces<UBInlinerInterface>();
  declarePromisedInterface<ConvertToLLVMPatternInterface, UBDialect>();
}

Operation *UBDialect::materializeConstant(OpBuilder &builder, Attribute value,
                                          Type type, Location loc) {
  if (auto attr = dyn_cast<PoisonAttr>(value))
    return PoisonOp::create(builder, loc, type, attr);

  return nullptr;
}

OpFoldResult PoisonOp::fold(FoldAdaptor /*adaptor*/) { return getValue(); }

#include "mlir/Dialect/UB/IR/UBOpsInterfaces.cpp.inc"

#define GET_ATTRDEF_CLASSES
#include "mlir/Dialect/UB/IR/UBOpsAttributes.cpp.inc"

#define GET_OP_CLASSES
#include "mlir/Dialect/UB/IR/UBOps.cpp.inc"
