//===-- HLFIRDialect.cpp --------------------------------------------------===//
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

#include "language/Compability/Optimizer/HLFIR/HLFIRDialect.h"
#include "language/Compability/Optimizer/Dialect/FIROps.h"
#include "language/Compability/Optimizer/Dialect/FIRType.h"
#include "language/Compability/Optimizer/HLFIR/HLFIROps.h"
#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/DialectImplementation.h"
#include "mlir/IR/Matchers.h"
#include "mlir/IR/OpImplementation.h"
#include "toolchain/ADT/SmallVector.h"
#include "toolchain/ADT/TypeSwitch.h"

#include "language/Compability/Optimizer/HLFIR/HLFIRDialect.cpp.inc"

#define GET_TYPEDEF_CLASSES
#include "language/Compability/Optimizer/HLFIR/HLFIRTypes.cpp.inc"

#define GET_ATTRDEF_CLASSES
#include "language/Compability/Optimizer/HLFIR/HLFIRAttributes.cpp.inc"

void hlfir::hlfirDialect::initialize() {
  addTypes<
#define GET_TYPEDEF_LIST
#include "language/Compability/Optimizer/HLFIR/HLFIRTypes.cpp.inc"
      >();
  addOperations<
#define GET_OP_LIST
#include "language/Compability/Optimizer/HLFIR/HLFIROps.cpp.inc"
      >();
}

// `expr` `<` `*` | bounds (`x` bounds)* `:` type [`?`] `>`
// bounds ::= `?` | int-lit
mlir::Type hlfir::ExprType::parse(mlir::AsmParser &parser) {
  if (parser.parseLess())
    return {};
  ExprType::Shape shape;
  if (parser.parseOptionalStar()) {
    if (parser.parseDimensionList(shape, /*allowDynamic=*/true))
      return {};
  } else if (parser.parseColon()) {
    return {};
  }
  mlir::Type eleTy;
  if (parser.parseType(eleTy))
    return {};
  const bool polymorphic = mlir::succeeded(parser.parseOptionalQuestion());
  if (parser.parseGreater())
    return {};
  return ExprType::get(parser.getContext(), shape, eleTy, polymorphic);
}

void hlfir::ExprType::print(mlir::AsmPrinter &printer) const {
  auto shape = getShape();
  printer << '<';
  if (shape.size()) {
    for (const auto &b : shape) {
      if (b >= 0)
        printer << b << 'x';
      else
        printer << "?x";
    }
  }
  printer << getEleTy();
  if (isPolymorphic())
    printer << '?';
  printer << '>';
}

bool hlfir::isFortranVariableType(mlir::Type type) {
  return toolchain::TypeSwitch<mlir::Type, bool>(type)
      .Case<fir::ReferenceType, fir::PointerType, fir::HeapType>([](auto p) {
        mlir::Type eleType = p.getEleTy();
        return mlir::isa<fir::BaseBoxType>(eleType) ||
               !fir::hasDynamicSize(eleType);
      })
      .Case<fir::BaseBoxType, fir::BoxCharType>([](auto) { return true; })
      .Case<fir::VectorType>([](auto) { return true; })
      .Default([](mlir::Type) { return false; });
}

bool hlfir::isFortranScalarCharacterType(mlir::Type type) {
  return isFortranScalarCharacterExprType(type) ||
         mlir::isa<fir::BoxCharType>(type) ||
         mlir::isa<fir::CharacterType>(
             fir::unwrapPassByRefType(fir::unwrapRefType(type)));
}

bool hlfir::isFortranScalarCharacterExprType(mlir::Type type) {
  if (auto exprType = mlir::dyn_cast<hlfir::ExprType>(type))
    return exprType.isScalar() &&
           mlir::isa<fir::CharacterType>(exprType.getElementType());
  return false;
}

bool hlfir::isFortranArrayCharacterExprType(mlir::Type type) {
  if (auto exprType = mlir::dyn_cast<hlfir::ExprType>(type))
    return exprType.isArray() &&
           mlir::isa<fir::CharacterType>(exprType.getElementType());

  return false;
}

bool hlfir::isFortranScalarNumericalType(mlir::Type type) {
  return fir::isa_integer(type) || fir::isa_real(type) ||
         fir::isa_complex(type);
}

bool hlfir::isFortranNumericalArrayObject(mlir::Type type) {
  if (isBoxAddressType(type))
    return false;
  if (auto arrayTy = mlir::dyn_cast<fir::SequenceType>(
          getFortranElementOrSequenceType(type)))
    return isFortranScalarNumericalType(arrayTy.getEleTy());
  return false;
}

bool hlfir::isFortranNumericalOrLogicalArrayObject(mlir::Type type) {
  if (isBoxAddressType(type))
    return false;
  if (auto arrayTy = mlir::dyn_cast<fir::SequenceType>(
          getFortranElementOrSequenceType(type))) {
    mlir::Type eleTy = arrayTy.getEleTy();
    return isFortranScalarNumericalType(eleTy) ||
           mlir::isa<fir::LogicalType>(eleTy);
  }
  return false;
}

bool hlfir::isFortranArrayObject(mlir::Type type) {
  if (isBoxAddressType(type))
    return false;
  return !!mlir::dyn_cast<fir::SequenceType>(
      getFortranElementOrSequenceType(type));
}

bool hlfir::isPassByRefOrIntegerType(mlir::Type type) {
  mlir::Type unwrappedType = fir::unwrapPassByRefType(type);
  return fir::isa_integer(unwrappedType);
}

bool hlfir::isI1Type(mlir::Type type) {
  if (mlir::IntegerType integer = mlir::dyn_cast<mlir::IntegerType>(type))
    if (integer.getWidth() == 1)
      return true;
  return false;
}

bool hlfir::isFortranLogicalArrayObject(mlir::Type type) {
  if (isBoxAddressType(type))
    return false;
  if (auto arrayTy = mlir::dyn_cast<fir::SequenceType>(
          getFortranElementOrSequenceType(type))) {
    mlir::Type eleTy = arrayTy.getEleTy();
    return mlir::isa<fir::LogicalType>(eleTy);
  }
  return false;
}

bool hlfir::isMaskArgument(mlir::Type type) {
  if (isBoxAddressType(type))
    return false;

  mlir::Type unwrappedType = fir::unwrapPassByRefType(fir::unwrapRefType(type));
  mlir::Type elementType = getFortranElementType(unwrappedType);
  if (unwrappedType != elementType)
    // input type is an array
    return mlir::isa<fir::LogicalType>(elementType);

  // input is a scalar, so allow i1 too
  return mlir::isa<fir::LogicalType>(elementType) || isI1Type(elementType);
}

bool hlfir::isPolymorphicObject(mlir::Type type) {
  if (auto exprType = mlir::dyn_cast<hlfir::ExprType>(type))
    return exprType.isPolymorphic();

  return fir::isPolymorphicType(type);
}

mlir::Value hlfir::genExprShape(mlir::OpBuilder &builder,
                                const mlir::Location &loc,
                                const hlfir::ExprType &expr) {
  mlir::IndexType indexTy = builder.getIndexType();
  toolchain::SmallVector<mlir::Value> extents;
  extents.reserve(expr.getRank());

  for (std::int64_t extent : expr.getShape()) {
    if (extent == hlfir::ExprType::getUnknownExtent())
      return {};
    extents.emplace_back(mlir::arith::ConstantOp::create(
        builder, loc, indexTy, builder.getIntegerAttr(indexTy, extent)));
  }

  fir::ShapeType shapeTy =
      fir::ShapeType::get(builder.getContext(), expr.getRank());
  fir::ShapeOp shape = fir::ShapeOp::create(builder, loc, shapeTy, extents);
  return shape.getResult();
}

bool hlfir::mayHaveAllocatableComponent(mlir::Type ty) {
  return fir::isPolymorphicType(ty) || fir::isUnlimitedPolymorphicType(ty) ||
         fir::isRecordWithAllocatableMember(hlfir::getFortranElementType(ty));
}

mlir::Type hlfir::getExprType(mlir::Type variableType) {
  hlfir::ExprType::Shape typeShape;
  bool isPolymorphic = fir::isPolymorphicType(variableType);
  mlir::Type type = getFortranElementOrSequenceType(variableType);
  if (auto seqType = mlir::dyn_cast<fir::SequenceType>(type)) {
    assert(!seqType.hasUnknownShape() && "assumed-rank cannot be expressions");
    typeShape.append(seqType.getShape().begin(), seqType.getShape().end());
    type = seqType.getEleTy();
  }
  return hlfir::ExprType::get(variableType.getContext(), typeShape, type,
                              isPolymorphic);
}

bool hlfir::isFortranIntegerScalarOrArrayObject(mlir::Type type) {
  if (isBoxAddressType(type))
    return false;

  mlir::Type unwrappedType = fir::unwrapPassByRefType(fir::unwrapRefType(type));
  mlir::Type elementType = getFortranElementType(unwrappedType);
  return mlir::isa<mlir::IntegerType>(elementType);
}
