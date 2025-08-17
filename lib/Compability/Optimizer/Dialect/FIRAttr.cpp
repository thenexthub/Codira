//===-- FIRAttr.cpp -------------------------------------------------------===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
//     http://www.apache.org/licenses/LICENSE-2.0
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
// Coding style: https://mlir.toolchain.org/getting_started/DeveloperGuide/
//
//===----------------------------------------------------------------------===//

#include "language/Compability/Optimizer/Dialect/FIRAttr.h"
#include "language/Compability/Optimizer/Dialect/FIRDialect.h"
#include "language/Compability/Optimizer/Dialect/Support/KindMapping.h"
#include "mlir/IR/AttributeSupport.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/DialectImplementation.h"
#include "toolchain/ADT/SmallString.h"
#include "toolchain/ADT/StringExtras.h"
#include "toolchain/ADT/TypeSwitch.h"

#include "language/Compability/Optimizer/Dialect/FIREnumAttr.cpp.inc"
#define GET_ATTRDEF_CLASSES
#include "language/Compability/Optimizer/Dialect/FIRAttr.cpp.inc"

using namespace fir;

namespace fir::detail {

struct RealAttributeStorage : public mlir::AttributeStorage {
  using KeyTy = std::pair<int, toolchain::APFloat>;

  RealAttributeStorage(int kind, const toolchain::APFloat &value)
      : kind(kind), value(value) {}
  RealAttributeStorage(const KeyTy &key)
      : RealAttributeStorage(key.first, key.second) {}

  static unsigned hashKey(const KeyTy &key) { return toolchain::hash_value(key); }

  bool operator==(const KeyTy &key) const {
    return key.first == kind &&
           key.second.compare(value) == toolchain::APFloatBase::cmpEqual;
  }

  static RealAttributeStorage *
  construct(mlir::AttributeStorageAllocator &allocator, const KeyTy &key) {
    return new (allocator.allocate<RealAttributeStorage>())
        RealAttributeStorage(key);
  }

  KindTy getFKind() const { return kind; }
  toolchain::APFloat getValue() const { return value; }

private:
  int kind;
  toolchain::APFloat value;
};

/// An attribute representing a reference to a type.
struct TypeAttributeStorage : public mlir::AttributeStorage {
  using KeyTy = mlir::Type;

  TypeAttributeStorage(mlir::Type value) : value(value) {
    assert(value && "must not be of Type null");
  }

  /// Key equality function.
  bool operator==(const KeyTy &key) const { return key == value; }

  /// Construct a new storage instance.
  static TypeAttributeStorage *
  construct(mlir::AttributeStorageAllocator &allocator, KeyTy key) {
    return new (allocator.allocate<TypeAttributeStorage>())
        TypeAttributeStorage(key);
  }

  mlir::Type getType() const { return value; }

private:
  mlir::Type value;
};
} // namespace fir::detail

//===----------------------------------------------------------------------===//
// Attributes for SELECT TYPE
//===----------------------------------------------------------------------===//

ExactTypeAttr fir::ExactTypeAttr::get(mlir::Type value) {
  return Base::get(value.getContext(), value);
}

mlir::Type fir::ExactTypeAttr::getType() const { return getImpl()->getType(); }

SubclassAttr fir::SubclassAttr::get(mlir::Type value) {
  return Base::get(value.getContext(), value);
}

mlir::Type fir::SubclassAttr::getType() const { return getImpl()->getType(); }

//===----------------------------------------------------------------------===//
// Attributes for SELECT CASE
//===----------------------------------------------------------------------===//

using AttributeUniquer = mlir::detail::AttributeUniquer;

ClosedIntervalAttr fir::ClosedIntervalAttr::get(mlir::MLIRContext *ctxt) {
  return AttributeUniquer::get<ClosedIntervalAttr>(ctxt);
}

UpperBoundAttr fir::UpperBoundAttr::get(mlir::MLIRContext *ctxt) {
  return AttributeUniquer::get<UpperBoundAttr>(ctxt);
}

LowerBoundAttr fir::LowerBoundAttr::get(mlir::MLIRContext *ctxt) {
  return AttributeUniquer::get<LowerBoundAttr>(ctxt);
}

PointIntervalAttr fir::PointIntervalAttr::get(mlir::MLIRContext *ctxt) {
  return AttributeUniquer::get<PointIntervalAttr>(ctxt);
}

//===----------------------------------------------------------------------===//
// RealAttr
//===----------------------------------------------------------------------===//

RealAttr fir::RealAttr::get(mlir::MLIRContext *ctxt,
                            const RealAttr::ValueType &key) {
  return Base::get(ctxt, key);
}

KindTy fir::RealAttr::getFKind() const { return getImpl()->getFKind(); }

toolchain::APFloat fir::RealAttr::getValue() const { return getImpl()->getValue(); }

//===----------------------------------------------------------------------===//
// FIR attribute parsing
//===----------------------------------------------------------------------===//

static mlir::Attribute parseFirRealAttr(FIROpsDialect *dialect,
                                        mlir::DialectAsmParser &parser,
                                        mlir::Type type) {
  int kind = 0;
  if (parser.parseLess() || parser.parseInteger(kind) || parser.parseComma()) {
    parser.emitError(parser.getNameLoc(), "expected '<' kind ','");
    return {};
  }
  KindMapping kindMap(dialect->getContext());
  toolchain::APFloat value(0.);
  if (parser.parseOptionalKeyword("i")) {
    // `i` not present, so literal float must be present
    double dontCare;
    if (parser.parseFloat(dontCare) || parser.parseGreater()) {
      parser.emitError(parser.getNameLoc(), "expected real constant '>'");
      return {};
    }
    auto fltStr = parser.getFullSymbolSpec()
                      .drop_until([](char c) { return c == ','; })
                      .drop_front()
                      .drop_while([](char c) { return c == ' ' || c == '\t'; })
                      .take_until([](char c) {
                        return c == '>' || c == ' ' || c == '\t';
                      });
    value = toolchain::APFloat(kindMap.getFloatSemantics(kind), fltStr);
  } else {
    // `i` is present, so literal bitstring (hex) must be present
    toolchain::StringRef hex;
    if (parser.parseKeyword(&hex) || parser.parseGreater()) {
      parser.emitError(parser.getNameLoc(), "expected real constant '>'");
      return {};
    }
    const toolchain::fltSemantics &sem = kindMap.getFloatSemantics(kind);
    unsigned int numBits = toolchain::APFloat::semanticsSizeInBits(sem);
    auto bits = toolchain::APInt(numBits, hex.drop_front(), 16);
    value = toolchain::APFloat(sem, bits);
  }
  return RealAttr::get(dialect->getContext(), {kind, value});
}

mlir::Attribute fir::FortranVariableFlagsAttr::parse(mlir::AsmParser &parser,
                                                     mlir::Type type) {
  if (mlir::failed(parser.parseLess()))
    return {};

  fir::FortranVariableFlagsEnum flags = {};
  if (mlir::failed(parser.parseOptionalGreater())) {
    auto parseFlags = [&]() -> mlir::ParseResult {
      toolchain::StringRef elemName;
      if (mlir::failed(parser.parseKeyword(&elemName)))
        return mlir::failure();

      auto elem = fir::symbolizeFortranVariableFlagsEnum(elemName);
      if (!elem)
        return parser.emitError(parser.getNameLoc(),
                                "Unknown fortran variable attribute: ")
               << elemName;

      flags = flags | *elem;
      return mlir::success();
    };
    if (mlir::failed(parser.parseCommaSeparatedList(parseFlags)) ||
        parser.parseGreater())
      return {};
  }

  return FortranVariableFlagsAttr::get(parser.getContext(), flags);
}

mlir::Attribute fir::parseFirAttribute(FIROpsDialect *dialect,
                                       mlir::DialectAsmParser &parser,
                                       mlir::Type type) {
  auto loc = parser.getNameLoc();
  toolchain::StringRef attrName;
  mlir::Attribute attr;
  mlir::OptionalParseResult result =
      generatedAttributeParser(parser, &attrName, type, attr);
  if (result.has_value())
    return attr;
  if (attrName.empty())
    return {}; // error reported by generatedAttributeParser

  if (attrName == ExactTypeAttr::getAttrName()) {
    mlir::Type type;
    if (parser.parseLess() || parser.parseType(type) || parser.parseGreater()) {
      parser.emitError(loc, "expected a type");
      return {};
    }
    return ExactTypeAttr::get(type);
  }
  if (attrName == SubclassAttr::getAttrName()) {
    mlir::Type type;
    if (parser.parseLess() || parser.parseType(type) || parser.parseGreater()) {
      parser.emitError(loc, "expected a subtype");
      return {};
    }
    return SubclassAttr::get(type);
  }
  if (attrName == PointIntervalAttr::getAttrName())
    return PointIntervalAttr::get(dialect->getContext());
  if (attrName == LowerBoundAttr::getAttrName())
    return LowerBoundAttr::get(dialect->getContext());
  if (attrName == UpperBoundAttr::getAttrName())
    return UpperBoundAttr::get(dialect->getContext());
  if (attrName == ClosedIntervalAttr::getAttrName())
    return ClosedIntervalAttr::get(dialect->getContext());
  if (attrName == RealAttr::getAttrName())
    return parseFirRealAttr(dialect, parser, type);

  parser.emitError(loc, "unknown FIR attribute: ") << attrName;
  return {};
}

//===----------------------------------------------------------------------===//
// FIR attribute pretty printer
//===----------------------------------------------------------------------===//

void fir::FortranVariableFlagsAttr::print(mlir::AsmPrinter &printer) const {
  printer << "<";
  printer << fir::stringifyFortranVariableFlagsEnum(this->getFlags());
  printer << ">";
}

void fir::printFirAttribute(FIROpsDialect *dialect, mlir::Attribute attr,
                            mlir::DialectAsmPrinter &p) {
  auto &os = p.getStream();
  if (auto exact = mlir::dyn_cast<fir::ExactTypeAttr>(attr)) {
    os << fir::ExactTypeAttr::getAttrName() << '<';
    p.printType(exact.getType());
    os << '>';
  } else if (auto sub = mlir::dyn_cast<fir::SubclassAttr>(attr)) {
    os << fir::SubclassAttr::getAttrName() << '<';
    p.printType(sub.getType());
    os << '>';
  } else if (mlir::dyn_cast_or_null<fir::PointIntervalAttr>(attr)) {
    os << fir::PointIntervalAttr::getAttrName();
  } else if (mlir::dyn_cast_or_null<fir::ClosedIntervalAttr>(attr)) {
    os << fir::ClosedIntervalAttr::getAttrName();
  } else if (mlir::dyn_cast_or_null<fir::LowerBoundAttr>(attr)) {
    os << fir::LowerBoundAttr::getAttrName();
  } else if (mlir::dyn_cast_or_null<fir::UpperBoundAttr>(attr)) {
    os << fir::UpperBoundAttr::getAttrName();
  } else if (auto a = mlir::dyn_cast_or_null<fir::RealAttr>(attr)) {
    os << fir::RealAttr::getAttrName() << '<' << a.getFKind() << ", i x";
    toolchain::SmallString<40> ss;
    a.getValue().bitcastToAPInt().toStringUnsigned(ss, 16);
    os << ss << '>';
  } else if (mlir::failed(generatedAttributePrinter(attr, p))) {
    // don't know how to print the attribute, so use a default
    os << "<(unknown attribute)>";
  }
}

//===----------------------------------------------------------------------===//
// FIROpsDialect
//===----------------------------------------------------------------------===//

void FIROpsDialect::registerAttributes() {
  addAttributes<ClosedIntervalAttr, ExactTypeAttr, LowerBoundAttr,
                PointIntervalAttr, RealAttr, SubclassAttr, UpperBoundAttr,
#define GET_ATTRDEF_LIST
#include "language/Compability/Optimizer/Dialect/FIRAttr.cpp.inc"
                >();
}
