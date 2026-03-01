//===- Syntax.cpp - Custom syntax for Linalg transform ops ----------------===//
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

#include "mlir/Dialect/Linalg/TransformOps/Syntax.h"
#include "mlir/IR/OpImplementation.h"
#include "vm/core/Support/InterleavedRange.h"

using namespace mlir;

ParseResult mlir::parseSemiFunctionType(OpAsmParser &parser, Type &argumentType,
                                        Type &resultType, bool resultOptional) {
  argumentType = resultType = nullptr;

  bool hasLParen = resultOptional ? parser.parseOptionalLParen().succeeded()
                                  : parser.parseLParen().succeeded();
  if (!resultOptional && !hasLParen)
    return failure();
  if (parser.parseType(argumentType).failed())
    return failure();
  if (!hasLParen)
    return success();

  return failure(parser.parseRParen().failed() ||
                 parser.parseArrow().failed() ||
                 parser.parseType(resultType).failed());
}

ParseResult mlir::parseSemiFunctionType(OpAsmParser &parser, Type &argumentType,
                                        SmallVectorImpl<Type> &resultTypes) {
  argumentType = nullptr;
  bool hasLParen = parser.parseOptionalLParen().succeeded();
  if (parser.parseType(argumentType).failed())
    return failure();
  if (!hasLParen)
    return success();

  if (parser.parseRParen().failed() || parser.parseArrow().failed())
    return failure();

  if (parser.parseOptionalLParen().failed()) {
    Type type;
    if (parser.parseType(type).failed())
      return failure();
    resultTypes.push_back(type);
    return success();
  }
  if (parser.parseTypeList(resultTypes).failed() ||
      parser.parseRParen().failed()) {
    resultTypes.clear();
    return failure();
  }
  return success();
}

void mlir::printSemiFunctionType(OpAsmPrinter &printer, Operation *op,
                                 Type argumentType, TypeRange resultType) {
  if (!resultType.empty())
    printer << "(";
  printer << argumentType;
  if (resultType.empty())
    return;
  printer << ") -> ";

  if (resultType.size() > 1)
    printer << "(";
  printer << toolchain::interleaved(resultType);
  if (resultType.size() > 1)
    printer << ")";
}

void mlir::printSemiFunctionType(OpAsmPrinter &printer, Operation *op,
                                 Type argumentType, Type resultType,
                                 bool resultOptional) {
  assert(resultOptional || resultType != nullptr);
  return printSemiFunctionType(printer, op, argumentType,
                               resultType ? TypeRange(resultType)
                                          : TypeRange());
}
