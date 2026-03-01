//===- Types.cpp ----------------------------------------------------------===//
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

#include "mlir/Tools/PDLL/AST/Types.h"
#include "mlir/Tools/PDLL/AST/Context.h"
#include <optional>

using namespace mlir;
using namespace mlir::pdll;
using namespace mlir::pdll::ast;

MLIR_DEFINE_EXPLICIT_TYPE_ID(mlir::pdll::ast::detail::AttributeTypeStorage)
MLIR_DEFINE_EXPLICIT_TYPE_ID(mlir::pdll::ast::detail::ConstraintTypeStorage)
MLIR_DEFINE_EXPLICIT_TYPE_ID(mlir::pdll::ast::detail::OperationTypeStorage)
MLIR_DEFINE_EXPLICIT_TYPE_ID(mlir::pdll::ast::detail::RangeTypeStorage)
MLIR_DEFINE_EXPLICIT_TYPE_ID(mlir::pdll::ast::detail::RewriteTypeStorage)
MLIR_DEFINE_EXPLICIT_TYPE_ID(mlir::pdll::ast::detail::TupleTypeStorage)
MLIR_DEFINE_EXPLICIT_TYPE_ID(mlir::pdll::ast::detail::TypeTypeStorage)
MLIR_DEFINE_EXPLICIT_TYPE_ID(mlir::pdll::ast::detail::ValueTypeStorage)

//===----------------------------------------------------------------------===//
// Type
//===----------------------------------------------------------------------===//

TypeID Type::getTypeID() const { return impl->typeID; }

Type Type::refineWith(Type other) const {
  if (*this == other)
    return *this;

  // Operation types are compatible if the operation names don't conflict.
  if (auto opTy = mlir::dyn_cast<OperationType>(*this)) {
    auto otherOpTy = mlir::dyn_cast<ast::OperationType>(other);
    if (!otherOpTy)
      return nullptr;
    if (!otherOpTy.getName())
      return *this;
    if (!opTy.getName())
      return other;

    return nullptr;
  }

  return nullptr;
}

//===----------------------------------------------------------------------===//
// AttributeType
//===----------------------------------------------------------------------===//

AttributeType AttributeType::get(Context &context) {
  return context.getTypeUniquer().get<ImplTy>();
}

//===----------------------------------------------------------------------===//
// ConstraintType
//===----------------------------------------------------------------------===//

ConstraintType ConstraintType::get(Context &context) {
  return context.getTypeUniquer().get<ImplTy>();
}

//===----------------------------------------------------------------------===//
// OperationType
//===----------------------------------------------------------------------===//

OperationType OperationType::get(Context &context,
                                 std::optional<StringRef> name,
                                 const ods::Operation *odsOp) {
  return context.getTypeUniquer().get<ImplTy>(
      /*initFn=*/function_ref<void(ImplTy *)>(),
      std::make_pair(name.value_or(""), odsOp));
}

std::optional<StringRef> OperationType::getName() const {
  StringRef name = getImplAs<ImplTy>()->getValue().first;
  return name.empty() ? std::optional<StringRef>()
                      : std::optional<StringRef>(name);
}

const ods::Operation *OperationType::getODSOperation() const {
  return getImplAs<ImplTy>()->getValue().second;
}

//===----------------------------------------------------------------------===//
// RangeType
//===----------------------------------------------------------------------===//

RangeType RangeType::get(Context &context, Type elementType) {
  return context.getTypeUniquer().get<ImplTy>(
      /*initFn=*/function_ref<void(ImplTy *)>(), elementType);
}

Type RangeType::getElementType() const {
  return getImplAs<ImplTy>()->getValue();
}

//===----------------------------------------------------------------------===//
// TypeRangeType
//===----------------------------------------------------------------------===//

bool TypeRangeType::classof(Type type) {
  RangeType range = mlir::dyn_cast<RangeType>(type);
  return range && mlir::isa<TypeType>(range.getElementType());
}

TypeRangeType TypeRangeType::get(Context &context) {
  return mlir::cast<TypeRangeType>(
      RangeType::get(context, TypeType::get(context)));
}

//===----------------------------------------------------------------------===//
// ValueRangeType
//===----------------------------------------------------------------------===//

bool ValueRangeType::classof(Type type) {
  RangeType range = mlir::dyn_cast<RangeType>(type);
  return range && mlir::isa<ValueType>(range.getElementType());
}

ValueRangeType ValueRangeType::get(Context &context) {
  return mlir::cast<ValueRangeType>(
      RangeType::get(context, ValueType::get(context)));
}

//===----------------------------------------------------------------------===//
// RewriteType
//===----------------------------------------------------------------------===//

RewriteType RewriteType::get(Context &context) {
  return context.getTypeUniquer().get<ImplTy>();
}

//===----------------------------------------------------------------------===//
// TupleType
//===----------------------------------------------------------------------===//

TupleType TupleType::get(Context &context, ArrayRef<Type> elementTypes,
                         ArrayRef<StringRef> elementNames) {
  assert(elementTypes.size() == elementNames.size());
  return context.getTypeUniquer().get<ImplTy>(
      /*initFn=*/function_ref<void(ImplTy *)>(), elementTypes, elementNames);
}
TupleType TupleType::get(Context &context, ArrayRef<Type> elementTypes) {
  SmallVector<StringRef> elementNames(elementTypes.size());
  return get(context, elementTypes, elementNames);
}

ArrayRef<Type> TupleType::getElementTypes() const {
  return getImplAs<ImplTy>()->getValue().first;
}

ArrayRef<StringRef> TupleType::getElementNames() const {
  return getImplAs<ImplTy>()->getValue().second;
}

//===----------------------------------------------------------------------===//
// TypeType
//===----------------------------------------------------------------------===//

TypeType TypeType::get(Context &context) {
  return context.getTypeUniquer().get<ImplTy>();
}

//===----------------------------------------------------------------------===//
// ValueType
//===----------------------------------------------------------------------===//

ValueType ValueType::get(Context &context) {
  return context.getTypeUniquer().get<ImplTy>();
}
