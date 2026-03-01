//===- ComplexDialect.cpp - MLIR Complex Dialect --------------------------===//
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

#include "mlir/Conversion/ConvertToLLVM/ToLLVMInterface.h"
#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Complex/IR/Complex.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/DialectImplementation.h"
#include "mlir/Transforms/InliningUtils.h"
#include "vm/core/ADT/TypeSwitch.h"

using namespace mlir;

#include "mlir/Dialect/Complex/IR/ComplexOpsDialect.cpp.inc"

namespace {
/// This class defines the interface for handling inlining for complex
/// dialect operations.
struct ComplexInlinerInterface : public DialectInlinerInterface {
  using DialectInlinerInterface::DialectInlinerInterface;
  /// All complex dialect ops can be inlined.
  bool isLegalToInline(Operation *, Region *, bool, IRMapping &) const final {
    return true;
  }
};
} // namespace

void complex::ComplexDialect::initialize() {
  addOperations<
#define GET_OP_LIST
#include "mlir/Dialect/Complex/IR/ComplexOps.cpp.inc"
      >();
  addAttributes<
#define GET_ATTRDEF_LIST
#include "mlir/Dialect/Complex/IR/ComplexAttributes.cpp.inc"
      >();
  declarePromisedInterface<ConvertToLLVMPatternInterface, ComplexDialect>();
  addInterfaces<ComplexInlinerInterface>();
}

Operation *complex::ComplexDialect::materializeConstant(OpBuilder &builder,
                                                        Attribute value,
                                                        Type type,
                                                        Location loc) {
  if (complex::ConstantOp::isBuildableWith(value, type)) {
    return complex::ConstantOp::create(builder, loc, type,
                                       toolchain::cast<ArrayAttr>(value));
  }
  return arith::ConstantOp::materialize(builder, value, type, loc);
}

#define GET_ATTRDEF_CLASSES
#include "mlir/Dialect/Complex/IR/ComplexAttributes.cpp.inc"

LogicalResult complex::NumberAttr::verify(
    ::toolchain::function_ref<::mlir::InFlightDiagnostic()> emitError,
    ::toolchain::APFloat real, ::toolchain::APFloat imag, ::mlir::Type type) {

  if (!toolchain::isa<ComplexType>(type))
    return emitError() << "complex attribute must be a complex type.";

  Type elementType = toolchain::cast<ComplexType>(type).getElementType();
  if (!toolchain::isa<FloatType>(elementType))
    return emitError()
           << "element type of the complex attribute must be float like type.";

  const auto &typeFloatSemantics =
      toolchain::cast<FloatType>(elementType).getFloatSemantics();
  if (&real.getSemantics() != &typeFloatSemantics)
    return emitError()
           << "type doesn't match the type implied by its `real` value";
  if (&imag.getSemantics() != &typeFloatSemantics)
    return emitError()
           << "type doesn't match the type implied by its `imag` value";

  return success();
}

void complex::NumberAttr::print(AsmPrinter &printer) const {
  printer << "<:" << toolchain::cast<ComplexType>(getType()).getElementType() << " "
          << getReal() << ", " << getImag() << ">";
}

Attribute complex::NumberAttr::parse(AsmParser &parser, Type odsType) {
  Type type;
  double real, imag;
  if (parser.parseLess() || parser.parseColon() || parser.parseType(type) ||
      parser.parseFloat(real) || parser.parseComma() ||
      parser.parseFloat(imag) || parser.parseGreater())
    return {};

  return NumberAttr::get(ComplexType::get(type), real, imag);
}
