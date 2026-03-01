//===- SMTAttributes.cpp - Implement SMT attributes -----------------------===//
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

#include "mlir/Dialect/SMT/IR/SMTAttributes.h"
#include "mlir/Dialect/SMT/IR/SMTDialect.h"
#include "mlir/Dialect/SMT/IR/SMTTypes.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/DialectImplementation.h"
#include "vm/core/ADT/TypeSwitch.h"

using namespace mlir;
using namespace mlir::smt;

//===----------------------------------------------------------------------===//
// BitVectorAttr
//===----------------------------------------------------------------------===//

LogicalResult BitVectorAttr::verify(
    function_ref<InFlightDiagnostic()> emitError,
    APInt value) { // NOLINT(performance-unnecessary-value-param)
  if (value.getBitWidth() < 1)
    return emitError() << "bit-width must be at least 1, but got "
                       << value.getBitWidth();
  return success();
}

std::string BitVectorAttr::getValueAsString(bool prefix) const {
  unsigned width = getValue().getBitWidth();
  SmallVector<char> toPrint;
  StringRef pref = prefix ? "#" : "";
  if (width % 4 == 0) {
    getValue().toString(toPrint, 16, false, false, false);
    // APInt's 'toString' omits leading zeros. However, those are critical here
    // because they determine the bit-width of the bit-vector.
    SmallVector<char> leadingZeros(width / 4 - toPrint.size(), '0');
    return (pref + "x" + Twine(leadingZeros) + toPrint).str();
  }

  getValue().toString(toPrint, 2, false, false, false);
  // APInt's 'toString' omits leading zeros
  SmallVector<char> leadingZeros(width - toPrint.size(), '0');
  return (pref + "b" + Twine(leadingZeros) + toPrint).str();
}

/// Parse an SMT-LIB formatted bit-vector string.
static FailureOr<APInt>
parseBitVectorString(function_ref<InFlightDiagnostic()> emitError,
                     StringRef value) {
  if (value[0] != '#')
    return emitError() << "expected '#'";

  if (value.size() < 3)
    return emitError() << "expected at least one digit";

  if (value[1] == 'b')
    return APInt(value.size() - 2, std::string(value.begin() + 2, value.end()),
                 2);

  if (value[1] == 'x')
    return APInt((value.size() - 2) * 4,
                 std::string(value.begin() + 2, value.end()), 16);

  return emitError() << "expected either 'b' or 'x'";
}

BitVectorAttr BitVectorAttr::get(MLIRContext *context, StringRef value) {
  auto maybeValue = parseBitVectorString(nullptr, value);

  assert(succeeded(maybeValue) && "string must have SMT-LIB format");
  return Base::get(context, *maybeValue);
}

BitVectorAttr
BitVectorAttr::getChecked(function_ref<InFlightDiagnostic()> emitError,
                          MLIRContext *context, StringRef value) {
  auto maybeValue = parseBitVectorString(emitError, value);
  if (failed(maybeValue))
    return {};

  return Base::getChecked(emitError, context, *maybeValue);
}

BitVectorAttr BitVectorAttr::get(MLIRContext *context, uint64_t value,
                                 unsigned width) {
  return Base::get(context, APInt(width, value));
}

BitVectorAttr
BitVectorAttr::getChecked(function_ref<InFlightDiagnostic()> emitError,
                          MLIRContext *context, uint64_t value,
                          unsigned width) {
  if (width < 64 && value >= (UINT64_C(1) << width)) {
    emitError() << "value does not fit in a bit-vector of desired width";
    return {};
  }
  return Base::getChecked(emitError, context, APInt(width, value));
}

Attribute BitVectorAttr::parse(AsmParser &odsParser, Type odsType) {
  toolchain::SMLoc loc = odsParser.getCurrentLocation();

  APInt val;
  if (odsParser.parseLess() || odsParser.parseInteger(val) ||
      odsParser.parseGreater())
    return {};

  // Requires the use of `quantified(<attr>)` in operation assembly formats.
  if (!odsType || !toolchain::isa<BitVectorType>(odsType)) {
    odsParser.emitError(loc) << "explicit bit-vector type required";
    return {};
  }

  unsigned width = toolchain::cast<BitVectorType>(odsType).getWidth();

  if (width > val.getBitWidth()) {
    // sext is always safe here, even for unsigned values, because the
    // parseOptionalInteger method will return something with a zero in the
    // top bits if it is a positive number.
    val = val.sext(width);
  } else if (width < val.getBitWidth()) {
    // The parser can return an unnecessarily wide result.
    // This isn't a problem, but truncating off bits is bad.
    unsigned neededBits =
        val.isNegative() ? val.getSignificantBits() : val.getActiveBits();
    if (width < neededBits) {
      odsParser.emitError(loc)
          << "integer value out of range for given bit-vector type " << odsType;
      return {};
    }
    val = val.trunc(width);
  }

  return BitVectorAttr::get(odsParser.getContext(), val);
}

void BitVectorAttr::print(AsmPrinter &odsPrinter) const {
  // This printer only works for the extended format where the MLIR
  // infrastructure prints the type for us. This means, the attribute should
  // never be used without `quantified` in an assembly format.
  odsPrinter << "<" << getValue() << ">";
}

Type BitVectorAttr::getType() const {
  return BitVectorType::get(getContext(), getValue().getBitWidth());
}

//===----------------------------------------------------------------------===//
// ODS Boilerplate
//===----------------------------------------------------------------------===//

#define GET_ATTRDEF_CLASSES
#include "mlir/Dialect/SMT/IR/SMTAttributes.cpp.inc"

void SMTDialect::registerAttributes() {
  addAttributes<
#define GET_ATTRDEF_LIST
#include "mlir/Dialect/SMT/IR/SMTAttributes.cpp.inc"
      >();
}
