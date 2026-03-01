//===- Utils.cpp - Transform dialect utilities ----------------------------===//
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

#include "mlir/Dialect/Transform/Utils/Utils.h"

#include "mlir/IR/OpDefinition.h"
#include "mlir/Interfaces/ViewLikeInterface.h"

using namespace mlir;
using namespace mlir::transform;

void mlir::transform::printPackedOrDynamicIndexList(
    OpAsmPrinter &printer, Operation *op, Value packed, Type packedType,
    OperandRange values, TypeRange valueTypes, DenseI64ArrayAttr integers) {
  if (packed) {
    assert(values.empty() && (!integers || integers.empty()) &&
           "expected no values/integers");
    printer << "*(" << packed;
    if (packedType) {
      printer << " : " << packedType;
    }
    printer << ")";
    return;
  }
  printDynamicIndexList(printer, op, values, integers, valueTypes);
}

ParseResult mlir::transform::parsePackedOrDynamicIndexList(
    OpAsmParser &parser, std::optional<OpAsmParser::UnresolvedOperand> &packed,
    Type &packedType, SmallVectorImpl<OpAsmParser::UnresolvedOperand> &values,
    SmallVectorImpl<Type> *valueTypes, DenseI64ArrayAttr &integers) {
  OpAsmParser::UnresolvedOperand packedOperand;
  if (parser.parseOptionalStar().succeeded()) {
    if (parser.parseLParen().failed() ||
        parser.parseOperand(packedOperand).failed())
      return failure();
    if (packedType && (parser.parseColonType(packedType).failed()))
      return failure();
    if (parser.parseRParen().failed())
      return failure();
    packed.emplace(packedOperand);
    integers = parser.getBuilder().getDenseI64ArrayAttr({});
    return success();
  }

  return parseDynamicIndexList(parser, values, integers, valueTypes);
}
