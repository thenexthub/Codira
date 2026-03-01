//===- SMTDialect.cpp - SMT dialect implementation ------------------------===//
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

#include "mlir/Dialect/SMT/IR/SMTDialect.h"
#include "mlir/Dialect/SMT/IR/SMTAttributes.h"
#include "mlir/Dialect/SMT/IR/SMTOps.h"
#include "mlir/Dialect/SMT/IR/SMTTypes.h"

using namespace mlir;
using namespace smt;

void SMTDialect::initialize() {
  registerAttributes();
  registerTypes();
  addOperations<
#define GET_OP_LIST
#include "mlir/Dialect/SMT/IR/SMT.cpp.inc"
      >();
}

Operation *SMTDialect::materializeConstant(OpBuilder &builder, Attribute value,
                                           Type type, Location loc) {
  // BitVectorType constants can materialize into smt.bv.constant
  if (auto bvType = dyn_cast<BitVectorType>(type)) {
    if (auto attrValue = dyn_cast<BitVectorAttr>(value)) {
      assert(bvType == attrValue.getType() &&
             "attribute and desired result types have to match");
      return BVConstantOp::create(builder, loc, attrValue);
    }
  }

  // BoolType constants can materialize into smt.constant
  if (auto boolType = dyn_cast<BoolType>(type)) {
    if (auto attrValue = dyn_cast<BoolAttr>(value))
      return BoolConstantOp::create(builder, loc, attrValue);
  }

  return nullptr;
}

#include "mlir/Dialect/SMT/IR/SMTDialect.cpp.inc"
#include "mlir/Dialect/SMT/IR/SMTEnums.cpp.inc"
