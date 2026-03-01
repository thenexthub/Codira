//===- PDLTypes.cpp - Pattern Descriptor Language Types -------------------===//
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

#include "mlir/Dialect/PDL/IR/PDLTypes.h"
#include "mlir/Dialect/PDL/IR/PDL.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/DialectImplementation.h"
#include "vm/core/ADT/TypeSwitch.h"

using namespace mlir;
using namespace mlir::pdl;

//===----------------------------------------------------------------------===//
// TableGen'd type method definitions
//===----------------------------------------------------------------------===//

#define GET_TYPEDEF_CLASSES
#include "mlir/Dialect/PDL/IR/PDLOpsTypes.cpp.inc"

//===----------------------------------------------------------------------===//
// PDLDialect
//===----------------------------------------------------------------------===//

void PDLDialect::registerTypes() {
  addTypes<
#define GET_TYPEDEF_LIST
#include "mlir/Dialect/PDL/IR/PDLOpsTypes.cpp.inc"
      >();
}

static Type parsePDLType(AsmParser &parser) {
  StringRef typeTag;
  {
    Type genType;
    auto parseResult = generatedTypeParser(parser, &typeTag, genType);
    if (parseResult.has_value())
      return genType;
  }

  // FIXME: This ends up with a double error being emitted if `RangeType` also
  // emits an error. We should rework the `generatedTypeParser` to better
  // support when the keyword is valid but the individual type parser itself
  // emits an error.
  parser.emitError(parser.getNameLoc(), "invalid 'pdl' type: `")
      << typeTag << "'";
  return Type();
}

//===----------------------------------------------------------------------===//
// PDL Types
//===----------------------------------------------------------------------===//

bool PDLType::classof(Type type) {
  return toolchain::isa<PDLDialect>(type.getDialect());
}

Type pdl::getRangeElementTypeOrSelf(Type type) {
  if (auto rangeType = toolchain::dyn_cast<RangeType>(type))
    return rangeType.getElementType();
  return type;
}

//===----------------------------------------------------------------------===//
// RangeType
//===----------------------------------------------------------------------===//

Type RangeType::parse(AsmParser &parser) {
  if (parser.parseLess())
    return Type();

  SMLoc elementLoc = parser.getCurrentLocation();
  Type elementType = parsePDLType(parser);
  if (!elementType || parser.parseGreater())
    return Type();

  if (toolchain::isa<RangeType>(elementType)) {
    parser.emitError(elementLoc)
        << "element of pdl.range cannot be another range, but got"
        << elementType;
    return Type();
  }
  return RangeType::get(elementType);
}

void RangeType::print(AsmPrinter &printer) const {
  printer << "<";
  (void)generatedTypePrinter(getElementType(), printer);
  printer << ">";
}

LogicalResult RangeType::verify(function_ref<InFlightDiagnostic()> emitError,
                                Type elementType) {
  if (!toolchain::isa<PDLType>(elementType) || toolchain::isa<RangeType>(elementType)) {
    return emitError()
           << "expected element of pdl.range to be one of [!pdl.attribute, "
              "!pdl.operation, !pdl.type, !pdl.value], but got "
           << elementType;
  }
  return success();
}
