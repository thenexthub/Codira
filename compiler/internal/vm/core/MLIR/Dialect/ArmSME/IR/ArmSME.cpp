//===- ArmSMEDialect.cpp - MLIR ArmSME dialect implementation -------------===//
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
// This file implements the ArmSME dialect and its operations.
//
//===----------------------------------------------------------------------===//

#include "mlir/Dialect/ArmSME/IR/ArmSME.h"
#include "mlir/Dialect/MemRef/IR/MemRef.h"
#include "mlir/IR/DialectImplementation.h"
#include "vm/core/ADT/TypeSwitch.h"

using namespace mlir;
using namespace mlir::arm_sme;

namespace mlir::arm_sme::detail {
LogicalResult verifyArmSMETileOpInterface(Operation *op) {
  return verifyOperationHasValidTileId(op);
}
} // namespace mlir::arm_sme::detail

//===----------------------------------------------------------------------===//
// Tablegen Definitions
//===----------------------------------------------------------------------===//

#include "mlir/Dialect/ArmSME/IR/ArmSMEDialect.cpp.inc"

#include "mlir/Dialect/ArmSME/IR/ArmSMEEnums.cpp.inc"

#include "mlir/Dialect/ArmSME/IR/ArmSMEOpInterfaces.cpp.inc"

#define GET_OP_CLASSES
#include "mlir/Dialect/ArmSME/IR/ArmSMEOps.cpp.inc"

#define GET_OP_CLASSES
#include "mlir/Dialect/ArmSME/IR/ArmSMEIntrinsicOps.cpp.inc"

#define GET_TYPEDEF_CLASSES
#include "mlir/Dialect/ArmSME/IR/ArmSMETypes.cpp.inc"

#define GET_ATTRDEF_CLASSES
#include "mlir/Dialect/ArmSME/IR/ArmSMEAttrDefs.cpp.inc"

void ArmSMEDialect::initialize() {
  addAttributes<
#define GET_ATTRDEF_LIST
#include "mlir/Dialect/ArmSME/IR/ArmSMEAttrDefs.cpp.inc"
      >();

  addOperations<
#define GET_OP_LIST
#include "mlir/Dialect/ArmSME/IR/ArmSMEOps.cpp.inc"
      ,
#define GET_OP_LIST
#include "mlir/Dialect/ArmSME/IR/ArmSMEIntrinsicOps.cpp.inc"
      >();
}
